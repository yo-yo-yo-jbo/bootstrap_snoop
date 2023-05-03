#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>

#define CLOSE_PORT(port)                do                                                        \
                                        {                                                         \
                                            if (MACH_PORT_NULL != (port))                         \
                                            {                                                     \
                                                mach_port_deallocate(mach_task_self(), port);     \
                                                (port) = MACH_PORT_NULL;                          \
                                            }                                                     \
                                        } while (false)

#define PRINT_LEN (140)

typedef struct
{
    mach_msg_header_t header;
    uint8_t buffer[1024];
    mach_msg_trailer_t trailer;
} custom_message_t;

static
void
start_stage(
    char* title
)
{
    int counter = 0;

    // Pretty-print
    for (counter = 0; '\0' != title[counter] && counter < PRINT_LEN; counter++)
    {
        putchar(title[counter]);
    }
    printf(" ...");
    for (; counter < PRINT_LEN; counter++)
    {
        putchar('.');
    }
    putchar(' ');

    // Flush
    fflush(stdout);
}

static
void
end_stage(void)
{
    // Pretty-print
    printf("[  \033[1;32mOK\033[0m  ]\n");

    // Flush
    fflush(stdout);
}

static
void
fail_stage(void)
{
    // Pretty-print
    printf("[  \033[1;31mFAIL\033[0m  ]\n");

    // Flush
    fflush(stdout);
}

static
bool
setup_new_port(
    mach_port_t* new_port
)
{
    bool result = false;
    mach_port_t port = MACH_PORT_NULL;
    kern_return_t kr = KERN_SUCCESS;

    // Create a new port
    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    if (KERN_SUCCESS != kr)
    {
        fail_stage();
        printf("mach_port_allocate() failed: 0x%.8x\n", kr);
        goto cleanup;
    }

    // Add send rights
    kr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
    if (KERN_SUCCESS != kr)
    {
        fail_stage();
        printf("mach_port_insert_right() failed: 0x%.8x\n", kr);
        goto cleanup;
    }

    // Success
    *new_port = port;
    port = MACH_PORT_NULL;
    result = true;

cleanup:

    // Free resources
    CLOSE_PORT(port);

    // Return result
    return result;
}

static
bool
switch_bootstrap_port(
    mach_port_t new_port,
    mach_port_t* old_port
)
{
    bool result = false;
    kern_return_t kr = KERN_SUCCESS;

    // Backup the old port if necessary
    if (NULL != old_port)
    {
        *old_port = bootstrap_port;
    }

    // Set the new port (API)
    kr = task_set_special_port(mach_task_self(), TASK_BOOTSTRAP_PORT, new_port);
    if (KERN_SUCCESS != kr)
    {
        fail_stage();
        printf("task_set_special_port() failed: 0x%.8x\n", kr);
        goto cleanup;
    }

    // Set the new port (global)
    bootstrap_port = new_port;

    // Success
    result = true;

cleanup:

    // Return result
    return result;
}

static
bool
parent_routine(
    mach_port_t new_port,
    mach_port_t old_port
)
{
    bool result = false;
    kern_return_t kr = KERN_SUCCESS;
    custom_message_t msg = { 0 };

    // Switch the port back
    start_stage("Switching bootstrap port back on parent process");
    if (!switch_bootstrap_port(old_port, NULL))
    {
        fail_stage();
        printf("switch_bootstrap_port() failed.\n");
        goto cleanup;
    }
    end_stage();

    // Receive messages
    start_stage("Waiting for messages");
    kr = mach_msg(&(msg.header), MACH_RCV_MSG, 0, sizeof(msg), new_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    if (KERN_SUCCESS != kr)
    {
        fail_stage();
        printf("mach_msg() failed: 0x%.8x.\n", kr);
        goto cleanup;
    }
    end_stage();

    // Print messagee
    printf("\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n===============================================\n");
    for (int i = 0; i < msg.header.msgh_size; i++)
    {
        printf("%.2x%c", msg.buffer[i], i % 16 == 15 ? '\n' : ' ');
    }
    printf("\n");

    // Success
    result = true;

cleanup:

    // Return result
    return result;
}

static
bool
child_routine(
    char* program,
    char** args
)
{
    // Execute
    execve(program, args, NULL);

    // Success
    return true;
}

int
main(
    int argc,
    char** argv
)
{
    bool result = false;
    mach_port_t new_port = MACH_PORT_NULL;
    mach_port_t old_port = MACH_PORT_NULL;
    pid_t child_pid = 0;

    // Check command-line arguments
    start_stage("Checking command-line arguments");
    if (2 > argc)
    {
        fail_stage();
        printf("Must have at least one argument.\n");
        goto cleanup;
    }
    end_stage();

    // Setup a new port
    start_stage("Setting up a fresh new port");
    if (!setup_new_port(&new_port))
    {
        fail_stage();
        printf("setup_new_port() failed.\n");
        goto cleanup;
    }
    end_stage();

    // Switch the bootstrap port
    start_stage("Switching bootstrap port");
    if (!switch_bootstrap_port(new_port, &old_port))
    {
        fail_stage();
        printf("switch_bootstrap_port() failed.\n");
        goto cleanup;
    }
    end_stage();

    // Fork
    start_stage("Forking");
    child_pid = fork();
    if (0 > child_pid)
    {
        fail_stage();
        printf("fork() failed.\n");
        goto cleanup;
    }

    // Handle parent
    if (0 != child_pid)
    {
        end_stage();
        if (!parent_routine(new_port, old_port))
        {
            goto cleanup;
        }
    }
    else
    {
        // Handle child
        new_port = MACH_PORT_NULL;
        if (!child_routine(argv[1], argv + 2))
        {
            goto cleanup;
        }
    }

    // Success
    result = true;

cleanup: 

    // Free resources
    CLOSE_PORT(new_port);

    // Indicate result
    return result ? 0 : -1;
}

