
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <json-c/json.h>

#include "menu_operation.h"
#include "json_Store.h"
#include "validation.h"

#define JSON_FILE "employee.json"

static pthread_mutex_t employee_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Internal worker functions */
static void *add_worker(void *arg);
static void *query_worker(void *arg);
static void *update_worker(void *arg);
static void *delete_worker(void *arg);

/* Run a worker in its own thread */
static void run_operation(void *(*worker)(void *))
{
    pthread_t thread;

    if (pthread_create(&thread, NULL, worker, NULL) != 0)
    {
        perror("pthread_create");
        return;
    }

    if (pthread_join(thread, NULL) != 0)
        perror("pthread_join");
}

/* Public functions called by main.c */
void *add_employee(void *arg)
{
    (void)arg;
    run_operation(add_worker);
    return NULL;
}

void *query_employee(void *arg)
{
    (void)arg;
    run_operation(query_worker);
    return NULL;
}

void *update_employee(void *arg)
{
    (void)arg;
    run_operation(update_worker);
    return NULL;
}

void *delete_employee(void *arg)
{
    (void)arg;
    run_operation(delete_worker);
    return NULL;
}

/* Find an employee using employee_ID */
static struct json_object *find_employee(
    struct json_object *employees,
    const char *id,
    size_t *found_index)
{
    size_t count = json_object_array_length(employees);

    for (size_t i = 0; i < count; i++)
    {
        struct json_object *employee =
            json_object_array_get_idx(employees, i);

        struct json_object *id_object = NULL;

        if (json_object_object_get_ex(
                employee, "employee_ID", &id_object))
        {
            char stored_id[32];

            snprintf(stored_id, sizeof(stored_id), "%s",
                     json_object_get_string(id_object));

            if (strcmp(stored_id, id) == 0)
            {
                if (found_index != NULL)
                    *found_index = i;

                return employee;
            }
        }
    }

    return NULL;
}

/* Get the employees array from the root object */
static struct json_object *get_employees(struct json_object *root)
{
    struct json_object *employees = NULL;

    if (root != NULL &&
        json_object_object_get_ex(root, "employees", &employees) &&
        json_object_is_type(employees, json_type_array))
    {
        return employees;
    }

    return NULL;
}

/* ADD EMPLOYEE */
static void *add_worker(void *arg)
{
    (void)arg;

    char id[32], name[100], age_text[16];
    char mobile[32], email[120], address[200];
    char attendance_text[8];

    printf("\n--- Add Employee ---\n");

    printf("Employee ID (1-2 digits): ");
    if (scanf("%31s", id) != 1 || !validate_id(id))
    {
        printf("Invalid employee ID.\n");
        return NULL;
    }

    printf("Name: ");
    if (scanf(" %99[^\n]", name) != 1 || !validate_name(name))
    {
        printf("Invalid name.\n");
        return NULL;
    }

    printf("Age: ");
    if (scanf("%15s", age_text) != 1 || !validate_age(age_text))
    {
        printf("Invalid age.\n");
        return NULL;
    }

    printf("Mobile (+91 followed by 10 digits): ");
    if (scanf("%31s", mobile) != 1 || !validate_mobile(mobile))
    {
        printf("Invalid mobile number.\n");
        return NULL;
    }

    printf("Email: ");
    if (scanf("%119s", email) != 1)
        return NULL;

    printf("Address: ");
    if (scanf(" %199[^\n]", address) != 1)
        return NULL;

    printf("Attendance (true/false): ");
    if (scanf("%7s", attendance_text) != 1 ||
        (strcmp(attendance_text, "true") != 0 &&
         strcmp(attendance_text, "false") != 0))
    {
        printf("Enter true or false.\n");
        return NULL;
    }

    pthread_mutex_lock(&employee_mutex);

    struct json_object *root = parse_json_file(JSON_FILE);
    struct json_object *employees = get_employees(root);

    if (employees == NULL)
    {
        printf("Could not load valid employee JSON.\n");
        if (root != NULL)
            json_object_put(root);

        pthread_mutex_unlock(&employee_mutex);
        return NULL;
    }

    if (find_employee(employees, id, NULL) != NULL)
    {
        printf("Employee ID already exists.\n");
        json_object_put(root);
        pthread_mutex_unlock(&employee_mutex);
        return NULL;
    }

    struct json_object *employee = json_object_new_object();
    struct json_object *details = json_object_new_object();
    struct json_object *contact = json_object_new_object();

    if (employee == NULL || details == NULL || contact == NULL)
    {
        printf("Memory allocation failed.\n");

        if (employee != NULL) json_object_put(employee);
        if (details != NULL) json_object_put(details);
        if (contact != NULL) json_object_put(contact);

        json_object_put(root);
        pthread_mutex_unlock(&employee_mutex);
        return NULL;
    }

    json_object_object_add(employee, "employee_ID",
                           json_object_new_int(atoi(id)));

    json_object_object_add(details, "name",
                           json_object_new_string(name));
    json_object_object_add(details, "age",
                           json_object_new_int(atoi(age_text)));
    json_object_object_add(details, "attendance",
                           json_object_new_boolean(
                               strcmp(attendance_text, "true") == 0));

    json_object_object_add(contact, "Mobile_number",
                           json_object_new_string(mobile));
    json_object_object_add(contact, "Email",
                           json_object_new_string(email));
    json_object_object_add(contact, "Address",
                           json_object_new_string(address));

    json_object_object_add(details, "contact_details", contact);
    json_object_object_add(employee, "employee_details", details);
    json_object_array_add(employees, employee);

    if (save_json_file(JSON_FILE, root))
        printf("Employee added successfully.\n");
    else
        printf("Failed to save employee data.\n");

    json_object_put(root);
    pthread_mutex_unlock(&employee_mutex);

    return NULL;
}

/* QUERY EMPLOYEE */
static void *query_worker(void *arg)
{
    (void)arg;

    char id[32];

    printf("\n--- Query Employee ---\n");
    printf("Enter employee ID: ");

    if (scanf("%31s", id) != 1)
        return NULL;

    pthread_mutex_lock(&employee_mutex);

    struct json_object *root = parse_json_file(JSON_FILE);
    struct json_object *employees = get_employees(root);

    if (employees == NULL)
    {
        printf("Could not load valid employee JSON.\n");
    }
    else
    {
        struct json_object *employee =
            find_employee(employees, id, NULL);

        if (employee != NULL)
            printf("%s\n", json_object_to_json_string_ext(
                employee, JSON_C_TO_STRING_PRETTY));
        else
            printf("Employee not found.\n");
    }

    if (root != NULL)
        json_object_put(root);

    pthread_mutex_unlock(&employee_mutex);

    return NULL;
}

/* UPDATE EMPLOYEE */
static void *update_worker(void *arg)
{
    (void)arg;

    char id[32], name[100], age_text[16];
    char mobile[32], email[120], address[200];

    printf("\n--- Update Employee ---\n");
    printf("Enter employee ID: ");

    if (scanf("%31s", id) != 1)
        return NULL;

    printf("New name: ");
    if (scanf(" %99[^\n]", name) != 1 || !validate_name(name))
    {
        printf("Invalid name.\n");
        return NULL;
    }

    printf("New age: ");
    if (scanf("%15s", age_text) != 1 || !validate_age(age_text))
    {
        printf("Invalid age.\n");
        return NULL;
    }

    printf("New mobile: ");
    if (scanf("%31s", mobile) != 1 || !validate_mobile(mobile))
    {
        printf("Invalid mobile number.\n");
        return NULL;
    }

    printf("New email: ");
    if (scanf("%119s", email) != 1)
        return NULL;

    printf("New address: ");
    if (scanf(" %199[^\n]", address) != 1)
        return NULL;

    pthread_mutex_lock(&employee_mutex);

    struct json_object *root = parse_json_file(JSON_FILE);
    struct json_object *employees = get_employees(root);

    struct json_object *employee =
        employees != NULL ? find_employee(employees, id, NULL) : NULL;

    if (employee == NULL)
    {
        printf("Employee not found or JSON is invalid.\n");

        if (root != NULL)
            json_object_put(root);

        pthread_mutex_unlock(&employee_mutex);
        return NULL;
    }

    struct json_object *details = NULL;
    struct json_object *contact = NULL;

    if (!json_object_object_get_ex(employee, "employee_details", &details) ||
        !json_object_object_get_ex(details, "contact_details", &contact))
    {
        printf("Employee record structure is invalid.\n");
        json_object_put(root);
        pthread_mutex_unlock(&employee_mutex);
        return NULL;
    }

    json_object_object_add(details, "name",
                           json_object_new_string(name));
    json_object_object_add(details, "age",
                           json_object_new_int(atoi(age_text)));
    json_object_object_add(contact, "Mobile_number",
                           json_object_new_string(mobile));
    json_object_object_add(contact, "Email",
                           json_object_new_string(email));
    json_object_object_add(contact, "Address",
                           json_object_new_string(address));

    if (save_json_file(JSON_FILE, root))
        printf("Employee updated successfully.\n");
    else
        printf("Failed to save employee changes.\n");

    json_object_put(root);
    pthread_mutex_unlock(&employee_mutex);

    return NULL;
}

/* DELETE EMPLOYEE */
static void *delete_worker(void *arg)
{
    (void)arg;

    char id[32];

    printf("\n--- Delete Employee ---\n");
    printf("Enter employee ID: ");

    if (scanf("%31s", id) != 1)
        return NULL;

    pthread_mutex_lock(&employee_mutex);

    struct json_object *root = parse_json_file(JSON_FILE);
    struct json_object *employees = get_employees(root);
    size_t index = 0;

    struct json_object *employee =
        employees != NULL ? find_employee(employees, id, &index) : NULL;

    if (employee == NULL)
    {
        printf("Employee not found or JSON is invalid.\n");

        if (root != NULL)
            json_object_put(root);

        pthread_mutex_unlock(&employee_mutex);
        return NULL;
    }

    json_object_array_del_idx(employees, index, 1);

    if (save_json_file(JSON_FILE, root))
        printf("Employee deleted successfully.\n");
    else
        printf("Failed to save employee deletion.\n");

    json_object_put(root);
    pthread_mutex_unlock(&employee_mutex);

    return NULL;
}

