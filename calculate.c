#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    /* Origin version*/
    FILE *fp = fopen("orig.txt", "r");
    FILE *output = fopen("output.txt", "w");
    if (!fp) {
        printf("ERROR opening input file orig.txt\n");
        exit(0);
    }
    char append[50], find[50];
    double orig_sum_a = 0.0, orig_sum_f = 0.0, orig_a, orig_f;
    for (int i = 0; i < 100; i++) {
        if (feof(fp)) {
            printf("ERROR: You need 100 datum instead of %d\n", i);
            printf("run 'make run' longer to get enough information\n\n");
            exit(0);
        }
        fscanf(fp, "%s %s %lf %lf\n", append, find,&orig_a, &orig_f);
        orig_sum_a += orig_a;
        orig_sum_f += orig_f;
    }
    fclose(fp);
    /* Origin Optimal Version*/
    fp = fopen("orig_opt.txt", "r");
    if (!fp) {
        fp = fopen("orig.txt", "r");
        if (!fp) {
            printf("ERROR opening input file opt.txt\n");
            exit(0);
        }
    }
    double opt_sum_a = 0.0, opt_sum_f = 0.0, opt_a, opt_f;
    for (int i = 0; i < 100; i++) {
        if (feof(fp)) {
            printf("ERROR: You need 100 datum instead of %d\n", i);
            printf("run 'make run' longer to get enough information\n\n");
            exit(0);
        }
        fscanf(fp, "%s %s %lf %lf\n", append, find,&opt_a, &opt_f);
        opt_sum_a += opt_a;
        opt_sum_f += opt_f;
    }
    fclose(fp);
    /* Optimal with align*/
    fp = fopen("opt.txt", "r");
    if (!fp) {
        fp = fopen("orig.txt", "r");
        if (!fp) {
            printf("ERROR opening input file opt.txt\n");
            exit(0);
        }
    }
    double opt2_sum_a = 0.0, opt2_sum_f = 0.0, opt2_a, opt2_f;
    for (int i = 0; i < 100; i++) {
        if (feof(fp)) {
            printf("ERROR: You need 100 datum instead of %d\n", i);
            printf("run 'make run' longer to get enough information\n\n");
            exit(0);
        }
        fscanf(fp, "%s %s %lf %lf\n", append, find,&opt2_a, &opt2_f);
        opt2_sum_a += opt2_a;
        opt2_sum_f += opt2_f;
    }
    fprintf(output, "append() %lf %lf %lf\n", orig_sum_a / 100.0,
            opt_sum_a / 100.0, opt2_sum_a / 100.0);
    fprintf(output, "findName() %lf %lf %lf", orig_sum_f / 100.0,
            opt_sum_f / 100.0, opt2_sum_f / 100.0);
    fclose(output);
    fclose(fp);
    return 0;
}
