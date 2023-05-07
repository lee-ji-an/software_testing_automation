#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define BUF_SIZE 1024

void print_init_setting(char* func_format);
void print_to_file(char* case_id, char** params, char * output);
void make_shell(char * test_file);

int param_cnt = 0;
char *func_name;
FILE* driver_fp;
FILE* shell_fp;
char *return_type;

int main(int argc, char* argv[])
{
    FILE* tc_fp;
    char func_format[BUF_SIZE];
    char line[BUF_SIZE];
    char *tc_token, *func_format_token;
    char *param_start_idx, *case_id;

    char * param_arr[param_cnt];
    char * expected_output;
    int idx;

    // argv[1]: testcase.txt // argv[2]: testfile.c
	if(argc != 3){
		printf("Usage: %s <testcase.txt> <testfile.c>\n", argv[0]);
		exit(1);
	}

    if ((tc_fp = fopen(argv[1], "r")) == NULL) {
        fputs("Cannot open test case file...\n", stderr);
        exit(1);
    }

    if ((driver_fp = fopen("./driver.c", "w+")) == NULL) {
        fputs("Failed to create driver.c...\n", stderr);
        exit(1);
    }

    if ((shell_fp = fopen("./run.sh", "w+")) == NULL) {
        fputs("Failed to create run.sh...\n", stderr);
        exit(1);
    }

    // make shell script file
    make_shell(argv[2]);

    // read format of the function to test
    while (1){
        fgets(func_format, sizeof(func_format), tc_fp);
        func_format[strlen(func_format) - 1] = '\0';
        if (func_format[0] != '/'){
            break;
        }
    }
    
    // print initial setting to c file
    print_init_setting(argv[2]);

    // parse function name and return type
    return_type = strtok(func_format, " ");
    func_name = strtok(strtok(NULL, ""), "(");

    // count the number of parameter
    func_format_token = strtok(NULL, ",");
    idx = 0;
    char *str = ")";
    if (strcmp(func_format_token, str) != 0){
        while ((func_format_token != NULL)) {
            param_cnt++;
            func_format_token = strtok(NULL, ",");
            idx++;
        }
    }

    
    while (fgets(line, sizeof(line), tc_fp) != NULL) {
        line[strlen(line) - 1] = '\0';
        // read id of test case
        case_id = strtok(line, " ");

        // read input values of test case
        tc_token = strtok(NULL, " ");
        
        for (int i = 0; i<param_cnt; i++){
            param_arr[i] = tc_token;
            tc_token = strtok(NULL, " ");
        }
        expected_output = tc_token;
        
        print_to_file(case_id, param_arr, expected_output);
    }

    fputs("}", driver_fp);
    fclose(tc_fp);
    fclose(driver_fp);

    return 0;
}

void print_init_setting(char* test_file){
    char init[BUF_SIZE] = {};
    strcat(init, "#include <stdio.h> \n#include <string.h> \n#include \"");
    strcat(init, test_file);
    strcat(init, "\"\n\n int main() {\n");
    fputs(init, driver_fp);
}

void print_to_file(char* case_id, char** params, char * output){
    int i;
    int flag;
    int ret_flag = 0;
    char print_string[BUF_SIZE*2] = {};

    strcat(print_string, "\tif (");
    strcat(print_string, func_name);
    strcat(print_string, "(");

    for (i = 0; i<param_cnt-1; i++) {
        flag = 0;
        if (((params[i][0]-'0') < 0) || ((params[i][0]-'0') > 9)) {
            strcat(print_string, "\'");
            flag = 1;
        }
        strcat(print_string, params[i]);
        if (flag){
            strcat(print_string, "\'");
        }
        strcat(print_string, ", ");
    }

    flag = 0;
    if (param_cnt > 0) {
        if (((params[i][0]-'0') < 0) || ((params[i][0]-'0') > 9)) {
            strcat(print_string, "\'");
            flag = 1;
        }
    strcat(print_string, params[i]);
    if (flag){
            strcat(print_string, "\'");
        }
    }
    
    strcat(print_string, ") == ");

    if (strcmp(return_type, "char") == 0){
        strcat(print_string, "\'");
        ret_flag = 1;
    }
    strcat(print_string, output);
    if (ret_flag){
        strcat(print_string, "\'");
    }
    strcat(print_string, ")    printf(\"test case ");
    strcat(print_string, case_id);
    strcat(print_string, ": pass\\n\");\n\telse printf(\"test case ");
    strcat(print_string, case_id);
    strcat(print_string, " : Fail\\n\");\n\n");

    fputs(print_string, driver_fp);
}

void make_shell(char * test_file){
    fputs("gcc -fprofile-arcs -ftest-coverage driver.c\n./a.out > report.txt\n", shell_fp);
    fputs("gcov driver.c -b -f >> report.txt\n", shell_fp);
    fputs("cat ", shell_fp);
    fputs(test_file, shell_fp);
    fputs(".gcov >> report.txt", shell_fp);
}