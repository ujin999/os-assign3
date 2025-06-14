#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Virtual Information Data Structure
typedef struct {
    unsigned int pid;
    unsigned int cr3;
    unsigned int virtual_address;
} VM_ENTRY;

// Outer Entry Data Structure
typedef struct {
    unsigned int pid;
    unsigned int entry_index;
    int valid;
    unsigned int inner_table_address;
} OUTER_ENTRY;

// Inner Entry Data Structure
typedef struct {
    unsigned int pid;
    unsigned int inner_table_address;
    unsigned int entry_index;
    int valid;
    unsigned int pfn;
} INNER_ENTRY;

// Result Data Structure
typedef struct {
    unsigned int pid;
    unsigned int memory_size;
    unsigned int required_pages;
    unsigned int *real_pgd_entries;
    int real_pgd_entries_cnt;
    int size_of_real_pgd_entries;
    unsigned int *fail_pgd_entries;
    int fail_pgd_entries_cnt;
    int size_of_fail_pgd_entries;
    unsigned int *allocated_pages;
    int allocated_pages_cnt;
    int size_of_allocated_pages;
    unsigned int num_of_pte_entries;
    unsigned int num_of_valid;
} RESULT;

// converts a decimal string to unsigned int.
unsigned int str_dec_to_uint(const char *dec) {
    unsigned int result;
    sscanf(dec, "%d", &result);

    return result;
}

// converts a hexadecimal string to unsigned int.
unsigned int str_hex_to_uint(const char *hex) {
    unsigned int result;
    sscanf(hex, "%x", &result);

    return result;
}

// insertion sort, sort by pid in virtual address array
void vm_sort_by_pid(VM_ENTRY arr[], int size) {
    for (int i = 1; i < size; i++) {
        int key = arr[i].pid;
        int j = i - 1;

        while (j >= 0 && arr[j].pid > key) {
            arr[j + 1].pid = arr[j].pid;
            j--;
        }
        arr[j + 1].pid = key;
    }
}

// insertion sort, sort by pid in outer entries array
void outer_sort_by_pid(OUTER_ENTRY arr[], int size) {
    for (int i = 1; i < size; i++) {
        int key = arr[i].pid;
        int j = i - 1;

        while (j >= 0 && arr[j].pid > key) {
            arr[j + 1].pid = arr[j].pid;
            j--;
        }
        arr[j + 1].pid = key;
    }
}

// insertion sort, sort by pid in inner entries array
void inner_sort_by_pid(INNER_ENTRY arr[], int size) {
    for (int i = 1; i < size; i++) {
        int key = arr[i].pid;
        int j = i - 1;

        while (j >= 0 && arr[j].pid > key) {
            arr[j + 1].pid = arr[j].pid;
            j--;
        }
        arr[j + 1].pid = key;
    }
}

// count number of pid in the virtual address array
int vm_count_pid(VM_ENTRY arr[], int size) {
    if (size == 0) return 0;

    vm_sort_by_pid(arr, size);

    int count = 1;
    for (int i = 1; i < size; i++) {
        if (arr[i].pid != arr[i - 1].pid) {
            count++;
        }
    }

    return count;
}

int main() {
    // file pointer declaration
    FILE *fp;
    
    // allocate memory for vm_entries
    int vm_size = 1000;
    int vm_cnt = 0;
    VM_ENTRY *vm_entries = malloc(sizeof(VM_ENTRY) * vm_size);

    // open vm.txt file
    fp = fopen("vm.txt", "r");
    // error handling
    if (!fp) {
        printf("fopen error");
    }

    // line buffer decalaration
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp)) {
        // value declaration
        unsigned int pid, cr3, virtual_address;

        // get pid from line
        char *token = strtok(buffer, ",");
        pid = str_dec_to_uint(token);

        // get cr3 from line
        token = strtok(NULL, ",");
        cr3 = str_hex_to_uint(token);

        // get virtual_address from line
        token = strtok(NULL, "\n");
        virtual_address = str_hex_to_uint(token);

        // if vm_cnt is equal to vm_size, reallocate double
        if (vm_cnt == vm_size) {
            vm_size *= 2;
            vm_entries = realloc(vm_entries, vm_size * sizeof(VM_ENTRY));
        }

        // set values in vm_entries
        vm_entries[vm_cnt++] = (VM_ENTRY){pid, cr3, virtual_address};    
    }

    // file close vm.txt file
    fclose(fp);

    // if process doesn't exist, exit
    if (vm_cnt == 0) {
        return 0;
    }

    // sort by pid && count pid at vm in virtual address
    int num_of_vm_pid_count = vm_count_pid(vm_entries, vm_cnt);

    // allocate memory for outer_entries
    int outer_size = 1000;
    int outer_cnt = 0;
    OUTER_ENTRY *outer_entries = malloc(sizeof(OUTER_ENTRY) * outer_size);

    // open outer.txt file
    fp = fopen("outer_table.txt", "r");
    // error handling
    if (!fp) {
        printf("fopen error");
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        // value declaration
        unsigned int pid, entry_index, inner_table_address;
        int valid;

        // get pid from line
        char *token = strtok(buffer, ",");
        pid = str_dec_to_uint(token);

        // get entry_index from line
        token = strtok(NULL, ",");
        entry_index = str_dec_to_uint(token);

        // get valid from line
        token = strtok(NULL, ",");
        valid = atoi(token);

        // get inner table address
        token = strtok(NULL, "\n");
        inner_table_address = str_hex_to_uint(token);

        // if outer_cnt is equal to outer_size, reallocate double
        if (outer_cnt == outer_size) {
            outer_size *= 2;
            outer_entries = realloc(outer_entries, outer_size * sizeof(OUTER_ENTRY));
        }

        // set values in outer_entries
        outer_entries[outer_cnt++] = (OUTER_ENTRY){pid, entry_index, valid, inner_table_address};
    }

    // file close outer.txt file
    fclose(fp);

    // sort by pid by outer entries
    outer_sort_by_pid(outer_entries, outer_cnt);

    // allocate memory for inner_entries
    int inner_size = 1000;
    int inner_cnt = 0;
    INNER_ENTRY *inner_entries = malloc(sizeof(INNER_ENTRY) * inner_size);

    // open inner.txt file
    fp = fopen("inner_table.txt", "r");
    // error handling
    if (!fp) {
        printf("fopen error");
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        // value declaration
        unsigned int pid, inner_table_address, entry_index, pfn;
        int valid;

        // get pid from line
        char *token = strtok(buffer, ",");
        pid = str_dec_to_uint(token);

        // get inner table address
        token = strtok(NULL, ",");
        inner_table_address = str_hex_to_uint(token);

        // get entry_index from line
        token = strtok(NULL, ",");
        entry_index = str_dec_to_uint(token);

        // get valid from line
        token = strtok(NULL, ",");
        valid = atoi(token);

        // get pfn
        token = strtok(NULL, "\n");
        pfn = str_hex_to_uint(token);

        // if inner_cnt is equal to inner_size, reallocate double
        if (inner_cnt == inner_size) {
            inner_size *= 2;
            inner_entries = realloc(inner_entries, inner_size * sizeof(INNER_ENTRY));
        }

        // set values in inner_entries
        inner_entries[inner_cnt++] = (INNER_ENTRY){pid, inner_table_address, entry_index, valid, pfn};
    }

    // file close inner.txt file
    fclose(fp);

    // sort by pid by inner entries
    inner_sort_by_pid(inner_entries, inner_cnt);

    // allocate memory for result
    RESULT *results = calloc(num_of_vm_pid_count, sizeof(RESULT));

    // initialize results real_pgd_entries and allocated_pages
    for (int i = 0; i < num_of_vm_pid_count; i++) {
        results[i].size_of_real_pgd_entries = 30;
        results[i].real_pgd_entries = malloc(results[i].size_of_real_pgd_entries * sizeof(unsigned int));
        results[i].size_of_fail_pgd_entries = 30;
        results[i].fail_pgd_entries = malloc(results[i].size_of_fail_pgd_entries * sizeof(unsigned int));
        results[i].size_of_allocated_pages = 30;
        results[i].allocated_pages = malloc(results[i].size_of_allocated_pages * sizeof(unsigned int));
    }

    // initialize results pid value
    results[0].pid = vm_entries[0].pid;
    int tmp_pid_count = 1;

    for (int i = 1; i < vm_cnt; i++) {
        if (vm_entries[i].pid != vm_entries[i - 1].pid) {
            results[tmp_pid_count].pid = vm_entries[i].pid;
            tmp_pid_count++;
        }
    }

    // value to find the next pid;
    int outer_pid_offset = 0;
    int inner_pid_offset = 0;
    int result_idx = 0;

    // iterate through all virtual memory entries
    for (int i = 0; i < vm_cnt; i++) {
        // add one page count
        results[result_idx].required_pages++;

        // virtual entry of index i
        VM_ENTRY vm_entry = vm_entries[i];
        RESULT *result = &results[result_idx];
        // get pgd_idx in vm_entry;
        unsigned int pgd_idx = vm_entry.virtual_address >> 22;

        // value to check if outer entries have corresponding pid values
        int is_in_outer = 0;

        // find out if the pgd_idx is in outer
        for (int j = outer_pid_offset; j < outer_cnt; j++) {
            OUTER_ENTRY outer_entry = outer_entries[j];

            // break if outer_entry.pid is over vm_entry.pid
            if (outer_entry.pid != vm_entry.pid) {
                break;
            }

            // check if outer table has pgd index
            if (outer_entry.entry_index == pgd_idx) {
                is_in_outer = 1;

                int is_pgd_entry = 0;

                // check if the result structure already has a corresponding pgd entry
                for (int j = 0; j < (*result).real_pgd_entries_cnt; j++) {
                    if ((*result).real_pgd_entries[j] == pgd_idx) {
                        is_pgd_entry = 1;
                        break;
                    }
                }

                // if there is no pdg entry, add it
                if (is_pgd_entry == 0) {
                    // increase allocated memory size if insufficient
                    if ((*result).real_pgd_entries_cnt == (*result).size_of_real_pgd_entries) {
                        (*result).size_of_real_pgd_entries *= 2;
                        (*result).real_pgd_entries = realloc((*result).real_pgd_entries, (*result).size_of_real_pgd_entries * sizeof(unsigned int));
                    }

                    // add pdg entry in result
                    (*result).real_pgd_entries[(*result).real_pgd_entries_cnt++] = pgd_idx;
                }
            }

            // if pgd_idx is in outer_entries and pgd is valid, set is_in_outer to 1.
            if (outer_entry.entry_index == pgd_idx && outer_entry.valid == 1) {

                // get the middle 10 bits of the virtual address
                unsigned int pt_idx = (vm_entry.virtual_address >> 12) & 0x3FF;

                // inner table traversal
                for (int k = inner_pid_offset; k < inner_cnt; k++) {
                    INNER_ENTRY inner_entry = inner_entries[k];

                    // break if pid is exceeded
                    if (inner_entry.pid != vm_entry.pid) {
                        break;
                    }

                    // if the inner table address and PTE index match, add PTE existence to the result
                    if (outer_entry.inner_table_address == inner_entry.inner_table_address
                        && inner_entry.entry_index == pt_idx) {
                            (*result).num_of_pte_entries++;
                        }

                    // if valid is 1, it is available
                    if (outer_entry.inner_table_address == inner_entry.inner_table_address
                        && inner_entry.entry_index == pt_idx
                        && inner_entry.valid == 1
                    ) { 
                        int physical_address = inner_entry.pfn;

                        int is_physical_address = 0;

                        // check for duplicate allocated pages in results
                        for (int l = 0; l < (*result).allocated_pages_cnt; l++) {
                            if ((*result).allocated_pages[l] == physical_address) {
                                is_physical_address = 1;
                                break;
                            }
                        }

                        // add physical address if not duplicated
                        if (is_physical_address == 0) {
                            (*result).allocated_pages[(*result).allocated_pages_cnt++] = physical_address;
                            (*result).num_of_valid++;
                        }
                    }
                }
            }
        }

        // entry does not exist on outer page
        if (is_in_outer == 0) {
            int is_pgd_entry = 0;

            // duplicate check if the corresponding pgd exists
            for (int j = 0; j < (*result).fail_pgd_entries_cnt; j++) {
                if ((*result).fail_pgd_entries[j] == pgd_idx) {
                    is_pgd_entry = 1;
                    break;
                }
            }

            // if not present in result, it is recorded as a pgd entry that failed to access.
            if (is_pgd_entry == 0) {
                // adjust size when dynamic allocation size is insufficient
                if ((*result).fail_pgd_entries_cnt == (*result).size_of_fail_pgd_entries) {
                    (*result).size_of_fail_pgd_entries *= 2;
                    (*result).fail_pgd_entries = realloc((*result).fail_pgd_entries, (*result).size_of_fail_pgd_entries * sizeof(unsigned int));
                }

                (*result).fail_pgd_entries[(*result).fail_pgd_entries_cnt++] = pgd_idx;
            }
        }

        // execute if pid changes in sorted array
        if ( i + 1 != vm_cnt && vm_entries[i + 1].pid != vm_entries[i].pid) {
            result_idx++;

            // adjust outer pid offset to match the corresponding pid
            for (int j = outer_pid_offset; j < outer_cnt; j++) {
                if (j + 1 != outer_cnt && outer_entries[j].pid != outer_entries[j + 1].pid) {
                    outer_pid_offset = j + 1;
                    break;
                }
            }

            // adjust inner pid offset to match the corresponding pid
            for (int j = inner_pid_offset; j < inner_cnt; j++) {
                if (j + 1 != inner_cnt && inner_entries[j].pid != inner_entries[j + 1].pid) {
                    inner_pid_offset = j + 1;
                    break;
                }
            }
        }
    }
    result_idx++;

    // open file
    fp = fopen("result.txt", "w");

    // handling error
    if (fp == NULL) {
        printf("fail to open");
        return 1;
    }

    // print for all results
    for (int i = 0; i < result_idx; i++) {
        fprintf(fp, "Process : %u, memory size : %u,  requires %u pages\n",
            results[i].pid,
            results[i].required_pages * 4096,
            results[i].required_pages);
    
        fprintf(fp, "#PGD entry(fail to access) : %d\n", results[i].fail_pgd_entries_cnt);
        fprintf(fp, "PGD entries(fail to access) : [");
        for (int j = 0; j < results[i].fail_pgd_entries_cnt; j++) {
            if (j == results[i].fail_pgd_entries_cnt - 1) {
                fprintf(fp, "%u", results[i].fail_pgd_entries[j]);
                break;
            }
            fprintf(fp, "%u, ", results[i].fail_pgd_entries[j]);
        }
        fprintf(fp, "]\n");
    
        fprintf(fp, "#PGD entry(success to access) : %d\n", results[i].real_pgd_entries_cnt);
        fprintf(fp, "PGD entries(success to access) : [");
        for (int j = 0; j < results[i].real_pgd_entries_cnt; j++) {
            if (j == results[i].real_pgd_entries_cnt - 1) {
                fprintf(fp, "%u", results[i].real_pgd_entries[j]);
                break;
            }
            fprintf(fp, "%u, ", results[i].real_pgd_entries[j]);
        }
        fprintf(fp, "]\n");
    
        fprintf(fp, "Process %d allocated pages : [", results[i].pid);
        for (int j = 0; j < results[i].allocated_pages_cnt; j++) {
            if (j == results[i].allocated_pages_cnt - 1) {
                fprintf(fp, "%#x", results[i].allocated_pages[j]);
                break;
            }
            fprintf(fp, "%#x, ", results[i].allocated_pages[j]);
        }
        fprintf(fp, "]\n");
    
        fprintf(fp, "TOTAL PTEntry : %u TOTAL valid : %u\n\n",
            results[i].num_of_pte_entries,
            results[i].num_of_valid);
    }

    // deallocate memory
    for (int i = 0; i < result_idx; i++) {
        free(results[i].real_pgd_entries);
        free(results[i].fail_pgd_entries);
        free(results[i].allocated_pages);
    }

    free(outer_entries);
    free(inner_entries);
    free(vm_entries);

    return 0;
}
