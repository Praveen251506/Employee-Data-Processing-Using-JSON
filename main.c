
#include <stdio.h>
#include "menu.h"
#include "menu_operation.h"

int main(void)
{
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

