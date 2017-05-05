#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include "linear.h"

#define THRESHOD_NUM 11
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

static char *line = NULL;
static int max_line_len;

void exit_input_error(int line_num){
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

static char* readline(FILE *input){
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL){
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

void read_problem(const char *filename, struct problem *prob, struct feature_node *x_space, double bias){
	int max_index, inst_max_index, i;
	size_t elements, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;

	if(fp == NULL){
		fprintf(stderr,"can't open input file %s\n", filename);
		exit(1);
	}

	prob->l = 0;
	elements = 0;
	max_line_len = 1024;
	line = Malloc(char, max_line_len);
	while(readline(fp)!=NULL){
		char *p = strtok(line, " \t");
		while(1){
			p = strtok(NULL," \t");
			if(p==NULL || *p=='\n') 
				break;
			++ elements;
		}
		++ elements;
		++ prob->l;
	}
	rewind(fp);

	prob->bias=bias;

	prob->y = Malloc(double, prob->l);
	prob->x = Malloc(struct feature_node *, prob->l);
	x_space = Malloc(struct feature_node, elements+prob->l);

	max_index = 0;
	j = 0;
	for(i=0; i<prob->l; i++){
		inst_max_index = 0;
		readline(fp);
		prob->x[i] = &x_space[j];
		label = strtok(line, " \t\n");
		if(label == NULL)
			exit_input_error(i+1);

		prob->y[i] = strtod(label, &endptr);
		if(endptr==label || *endptr!='\0')
			exit_input_error(i+1);

		while(1){
			idx = strtok(NULL,  ":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			x_space[j].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
				exit_input_error(i+1);
			else
				inst_max_index = x_space[j].index;

			errno = 0;
			x_space[j].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i+1);

			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;

		if(prob->bias >= 0)
			x_space[j++].value = prob->bias;

		x_space[j++].index = -1;
	}

	if(prob->bias >= 0){
		prob->n = max_index + 1;
		for(i=1; i<prob->l; i++)
			(prob->x[i]-2)->index = prob->n;
		x_space[j-2].index = prob->n;
	}
	else
		prob->n = max_index;

	fclose(fp);
}

int main(int argc, char *argv[]){
	struct feature_node *x_space = NULL;
	struct parameter param;
	struct problem prob;
	struct model* md;
	struct timeval start, finish;

	char model_file_name[] = "../plain/svm_model";
	char train_file_name[] = "../plain/svm_train";
	char test_file_name[] = "../plain/svm_test";

	//for(double c=0.4; c<=0.5; c+=0.02){
	param.solver_type = L2R_L2LOSS_SVC_DUAL;
	param.C = 0.5;
	param.eps = 0.00001;
	param.p = 0.1;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	param.init_sol = NULL;
	double bias = -1;
	double predict;
	double *decision;
	double timeuse;
	double tp[THRESHOD_NUM] = {0};
	double fp[THRESHOD_NUM] = {0};
	double tn[THRESHOD_NUM] = {0};
	double fn[THRESHOD_NUM] = {0};
	double th[] = {-8, -4, -2, -1, -0.5, 0, 0.5, 1, 2, 4, 8};

	void (*print_func)(const char*) = NULL;
	set_print_string_function(print_func);
	
	read_problem(train_file_name, &prob, x_space, bias);
	gettimeofday(&start, NULL);
	md = train(&prob, &param);
	gettimeofday(&finish, NULL);
	timeuse = (finish.tv_sec-start.tv_sec)+(finish.tv_usec-start.tv_usec)/1000000.0;
	printf("Training Time: %f\n", timeuse);
	save_model(model_file_name, md);

	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);

	read_problem(test_file_name, &prob, x_space, bias);
	double count = 0;
	decision = (double *)malloc(sizeof(double)*prob.l);
	gettimeofday(&start, NULL);
	for(int i=0; i<prob.l; ++i){
		predict = predict_values(md, prob.x[i], &(decision[i]));
		if(int(predict)==prob.y[i]) ++ count;
		for(int j=0; j<THRESHOD_NUM; ++j){
			if(prob.y[i]==0 && decision[i]>th[j]) ++(tp[j]);
			else if(prob.y[i]==0 && decision[i]<=th[j]) ++(fn[j]);
			else if(prob.y[i]==1 && decision[i]>th[j]) ++(fp[j]);
			else if(prob.y[i]==1 && decision[i]<=th[j]) ++(tn[j]);
		}
	}
	gettimeofday(&finish, NULL);
	timeuse = (finish.tv_sec-start.tv_sec)+(finish.tv_usec-start.tv_usec)/1000000.0;
	printf("Testing Time: %f\n", timeuse);
	printf("Acc: %0.4f\n", count/prob.l);
	FILE *out = fopen("svm_test_data", "w");
	for(int i=0; i<THRESHOD_NUM; ++i){
		printf("TH:%0.1f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f\n", th[i], tp[i]/prob.l, tn[i]/prob.l, fp[i]/prob.l, fn[i]/prob.l, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i]))));
		fprintf(out, "TH:%0.1f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f\n", th[i], tp[i]/prob.l, tn[i]/prob.l, fp[i]/prob.l, fn[i]/prob.l, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i]))));
	}
	fclose(out);

	free_and_destroy_model(&md);
	destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);
	free(decision);
	//}

	return 0;
}













