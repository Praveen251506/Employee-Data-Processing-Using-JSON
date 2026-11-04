
# 👩‍💻 Employee Data Processing Using JSON

### A little C, a little JSON, and a Raspberry Pi doing the work. 🥧

[![Language](https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Data](https://img.shields.io/badge/Data-JSON-orange?style=for-the-badge&logo=json)](https://www.json.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Raspberry%20Pi-red?style=for-the-badge&logo=raspberrypi)](https://www.raspberrypi.com/)
[![Library](https://img.shields.io/badge/Library-JSON--C-green?style=for-the-badge)](https://github.com/json-c/json-c)

**Maven Silicon Mini Project**

*Making employee records easier to manage --- one JSON object at a
time.*


------------------------------------------------------------------------

## 🌟 What's this project about?

Imagine managing employee records without opening a spreadsheet every
time. That's the idea behind this mini-project!

This is a **menu-driven employee data management application written in
C**. It stores employee information in a JSON file and lets an
authorised user work with those records through a simple terminal menu.

The project brings together C programming, JSON data handling, file
operations, input validation, threads, and RFID-based access control.

## 🎯 What can it do?

Once access is granted, the menu lets you:

  ------------------------------------------------------------------------
              Menu             Feature               What happens?
  ---------------------------- --------------------- ---------------------
              `1`              🔎 Query Employee     Find an employee
                                                     using their ID.

              `2`              ✏️ Update Employee    Modify an existing
                                                     employee's details.

              `3`              ➕ Add Employee       Add a new employee
                                                     record.

              `4`              📋 Display All        Display all employee
                                                     records stored in the
                                                     JSON file.

              `5`              🗑️ Delete Employee    Remove an employee
                                                     using their ID.

              `6`              🚪 Exit               Leave the
                                                     application.
  ------------------------------------------------------------------------

## 🔐 A quick check at the door: RFID

Before the employee menu appears, the application calls the existing
RFID function and gets the scanned card UID.

-   ✅ **Matching UID:** authentication succeeds and the menu opens.
-   ❌ **Different UID:** access is denied and the application exits.

The authorised UID is configured in the C source. The RFID reader
implementation is maintained separately; the employee application
expects the RFID function to be available and linked when building.

> **Hardware note:** This repository's employee-management source files
> are separate. The RC522/SPI reader code is not replaced by the JSON
> employee-management code.

## 🧩 How the pieces fit together

The project is split into separate C source and header files so each
part has a clear responsibility. That makes the code easier to read,
test, and maintain.

  -----------------------------------------------------------------------
  File                                Its job
  ----------------------------------- -----------------------------------
  `main.c`                            Starts the application and controls
                                      the main program flow.

  `menu.c` / `menu.h`                 Displays the employee menu and
                                      provides its declarations.

  `menu_operation.c` /                Contains the employee operations
  `menu_operation.h`                  such as query, update, add,
                                      display, and delete.

  `json_Store.c` / `json_Store.h`     Reads employee data from JSON and
                                      saves changes back to the file.

  `validation.c` / `validation.h`     Checks user input before it is used
                                      by the application.

  `employee.json`                     Stores the employee records.
  -----------------------------------------------------------------------

The RFID function/source is separate from these employee modules.
Include the appropriate RFID source file or compiled object when linking
the application.

## 🗂️ The data: `employee.json`

JSON (JavaScript Object Notation) is a lightweight text format for
storing structured data. It represents information using objects,
arrays, and values such as strings, numbers, and Booleans.

This project keeps employee records in an `employees` array. Each record
contains an employee ID and an object holding the employee's details.

A simplified example:

``` json
{
  "employees": [
    {
      "employee_ID": 1,
      "employee_details": {
        "name": "Sample Employee",
        "age": 25,
        "attendance": true,
        "contact_details": {
          "Mobile_number": "+911234567890",
          "Email": "sample@example.com",
          "Address": "Sample address"
        }
      }
    }
  ]
}
```

The example is for illustration. Keep the field names and structure
consistent with the `employee.json` format expected by your source code.

### 🔄 What happens to the data?

-   **Read:** JSON-C parses the JSON file so the C program can access
    employee records.
-   **Process:** The selected operation searches or changes the employee
    data.
-   **Save:** When records are added, updated, or deleted, the modified
    JSON data is written back to the file.

No database server needed --- the JSON file is the data store.

## 🛠️ Built with

-   **C** --- application logic
-   **JSON-C** --- parsing and writing JSON
-   **GCC** --- compiling the C source files
-   **pthread** --- thread support used by the employee-operation code
-   **Linux / Raspberry Pi OS** --- development and runtime environment
-   **RFID reader integration** --- access check using the existing RFID
    function

## 🚀 Let's get it running

### 1. Get the project

``` bash
git clone <your-repository-url>
cd Employee-Data-Processing-Using-JSON
```

### 2. Install the required packages

On Raspberry Pi OS or another Debian-based Linux system:

``` bash
sudo apt update
sudo apt install build-essential pkg-config libjson-c-dev
```

### 3. Check your project files

Keep the separate `.c` and `.h` files together with `employee.json`. The
JSON file must be in the directory from which you run the application
because the program uses the relative filename `employee.json`.

### 4. Compile the employee modules

If the RFID implementation is in a separate source file named `rfid.c`,
compile all the modules together:

``` bash
gcc -Wall -Wextra main.c menu.c menu_operation.c \
    json_Store.c validation.c rfid.c \
    $(pkg-config --cflags --libs json-c) \
    -pthread -o employee_app
```

Replace `rfid.c` with the actual filename that provides `rfid_main()`.
If your RFID code is already compiled into an object file, use that
object file instead of `rfid.c`, for example:

``` bash
gcc -Wall -Wextra main.c menu.c menu_operation.c \
    json_Store.c validation.c rfid.o \
    $(pkg-config --cflags --libs json-c) \
    -pthread -o employee_app
```

**Tiny but important linker detail:** a declaration of `rfid_main()`
tells the compiler what the function looks like; it does not include the
function implementation. The source file, object file, or library that
implements it must be linked into the final executable.

### 5. Run it!

``` bash
./employee_app
```

Scan an authorised RFID card, then choose an operation from the menu.
Follow the prompts in the terminal and choose `6` when you're ready to
exit.

## 🧪 Input checks

The validation module checks the inputs used for employee records. In
the current project version, the checks include:

-   Employee ID: digits only, with the configured length limit.
-   Name: letters and spaces, with at least one letter.
-   Age: digits only and within the accepted age range.
-   Mobile number: `+91` followed by 10 digits.
-   Attendance: `true` or `false` when adding a record.

Email is currently accepted as text; the code does not perform full
email-address validation.

## 🧠 What you'll learn

By exploring this project, you'll get hands-on practice with:

-   Splitting a C application across `.c` and `.h` files.
-   Calling functions across multiple source files.
-   Parsing and generating JSON with JSON-C.
-   Reading from and writing to files.
-   Building menu-driven CRUD operations.
-   Validating user input.
-   Using pthreads and a mutex.
-   Linking an existing hardware-facing function into an application.

## 📁 Suggested repository layout

``` text
Employee-Data-Processing-Using-JSON/
│
├── main.c
├── menu.c
├── menu.h
├── menu_operation.c
├── menu_operation.h
├── json_Store.c
├── json_Store.h
├── validation.c
├── validation.h
├── employee.json
└── README.md
```

Add the RFID source/header files too if they are part of the repository
and you are able to share them. Otherwise, keep them separately and make
sure the build command links the implementation.

## ⚠️ Before you push to GitHub

-   Use sample or dummy employee records in `employee.json`.
-   Do not publish real phone numbers, email addresses, or home
    addresses.
-   Keep a backup of the JSON file before testing update and delete
    operations.
-   The hardcoded RFID UID is suitable for a learning demo, not
    production-grade access control.
-   The application expects a valid JSON file with the structure used by
    the program.

## 💬 The idea behind the project

This mini-project is a practical way to connect concepts that are often
learned separately: C functions and header files, structured JSON data,
file handling, validation, threads, and hardware authentication.

**Thanks for checking it out!** ⭐

If you find something interesting or have an idea to improve it, feel
free to explore, experiment, and build on it.

::: {align="center"}
**Made with C, JSON, and curiosity.** 💙
:::
