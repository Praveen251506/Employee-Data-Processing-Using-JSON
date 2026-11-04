/*
 * Employee Data Processing with RFID authentication
 *
 * The employee application files have been combined into this file.
 * The actual RFID reader implementation must provide:
 *
 *     uint64_t rfid_main(void);
 *
 * AUTHORIZED_RFID_UID must be replaced with the UID of your authorized card.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <pthread.h>
#include <json-c/json.h>

#define JSON_FILE "employee.json"
#define AUTHORIZED_RFID_UID UINT64_C(2064439950)

/* Function implemented in the separately compiled RFID code. */
uint64_t rfid_main(void);

/* Declarations formerly supplied by the project's header files. */
void display_menu(void);

void *query_employee(void *arg);
void *add_employee(void *arg);
void *update_employee(void *arg);
void *delete_employee(void *arg);

struct json_object *parse_json_file(const char *filename);
int save_json_file(const char *filename, struct json_object *root);

int validate_id(const char *id);
int validate_name(const char *name);
int validate_age(const char *age);
int validate_mobile(const char *mobile);


/* ================= JSON FILE HANDLING ================= */
struct json_object *parse_json_file(const char *filename)
{
	FILE *fp = fopen(filename,"r");

	if(fp == NULL)
		return NULL;

	fseek(fp,0,SEEK_END);
	long size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char *buffer = malloc(size+1);

	if(buffer == NULL) {
		fclose(fp);
		return NULL;
	}

	fread(buffer,1,size,fp);
	buffer[size] ='\0';

	fclose(fp);

	struct json_object *root = json_tokener_parse(buffer);

	free(buffer);

	return root;
}


/* Save JSON file */
int save_json_file(const char *filename,struct json_object *root)

{
    const char *json_string = json_object_to_json_string_ext( root, JSON_C_TO_STRING_PRETTY);


    FILE *fp = fopen(filename,"w");

    if(fp ==NULL)
	    return 0;

    fprintf(fp,"%s\n",json_string);

    fclose(fp);
    
    return 1;

}

/* ================= INPUT VALIDATION ================= */
/* Validate ID: digits only, maximum 2 digits */
int validate_id(const char *id)
{
    if (id == NULL || strlen(id) == 0 || strlen(id) > 2)
        return 0;

    for (int i = 0; id[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)id[i]))
            return 0;
    }

    return 1;
}

/* Validate name: letters and spaces only */
int validate_name(const char *name)
{
    if (name == NULL || strlen(name) == 0)
        return 0;

    int has_character = 0;

    for (int i = 0; name[i] != '\0'; i++)
    {
        if (isalpha((unsigned char)name[i]))
            has_character = 1;
        else if (name[i] != ' ')
            return 0;
    }

    return has_character;
}

/* Validate age: digits only, between 1 and 149 */
int validate_age(const char *age)
{
    if (age == NULL || strlen(age) == 0)
        return 0;

    for (int i = 0; age[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)age[i]))
            return 0;
    }

    int value = atoi(age);

    return (value > 0 && value < 150);
}

/* Validate mobile: +91 followed by 10 digits */
int validate_mobile(const char *mobile)
{
    if (mobile == NULL || strlen(mobile) != 13)
        return 0;

    if (mobile[0] != '+' ||
        mobile[1] != '9' ||
        mobile[2] != '1')
        return 0;

    for (int i = 3; mobile[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)mobile[i]))
            return 0;
    }

    return 1;
}

/* ================= MENU DISPLAY ================= */
void display_menu(void)
{
    printf("\n========== Employee Data Processing ==========\n");
    printf("1. Query Employee\n");
    printf("2. Update Employee\n");
    printf("3. Add Employee\n");
    printf("4. Delete Employee\n");
    printf("5. Exit\n");
    printf("==============================================\n");
    
}

/* ================= EMPLOYEE OPERATIONS ================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <json-c/json.h>
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


/* ================= RFID AUTHENTICATION AND MAIN ================= */

/* RFID authentication wrapper.
 * rfid_main() is supplied by the RFID code already on your Raspberry Pi.
 */
static uint64_t read_rfid_id(void)
{
    printf("\nScan RFID card...\n");

    uint64_t scanned_rfid = rfid_main();

    printf("Scanned Employee ID: %" PRIu64 "\n", scanned_rfid);

    return scanned_rfid;
}

int main(void)
{
    uint64_t scanned_id = read_rfid_id();

    if (scanned_id != AUTHORIZED_RFID_UID)
    {
        printf("Access Denied! Invalid RFID card.\n");
        return 0;
    }

    printf("RFID Authentication Successful!\n");

    int choice;

    while (1)
    {
        display_menu();

        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");

            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            continue;
        }

        /* Remove the leftover newline before worker input */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;

        if (choice == 5)
        {
            printf("Exiting Employee Data Processing.\n");
            break;
        }

        switch (choice)
        {
            case 1:
                query_employee(NULL);
                break;

            case 2:
                update_employee(NULL);
                break;

            case 3:
                add_employee(NULL);
                break;

            case 4:
                delete_employee(NULL);
                break;

            default:
                printf("Invalid choice. Select 1 to 5.\n");
        }
    }

    return 0;
}
