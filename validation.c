#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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
