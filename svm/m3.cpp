#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include "linear.h"

#define RAND_MIN_TH 15
#define RAND_MAX_TH 2
#define YC_MIN_TH 16
#define YC_MAX_TH 0
#define THRESHOD_NUM 11
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void printUsage(){
	printf("Usage: m3 [option] [pos num] [neg num] [test mod]\n");
	printf("Options:\n");
	printf("\trand\trandom sub-classes\n");
	printf("\tyc\theuristic sub-classes\n");
	printf("Test Mods:\n");
	printf("\tgrid\tgrid search the best min max threshod\n");
	printf("\teval\tevaluate TPR FPR P R F1\n");
}

void print_null(const char *s) {}

void exit_input_error(int line_num){
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

static char* readline(FILE *input, char **line, int *max_line_len){
	int len;

	if(fgets(*line,*max_line_len,input) == NULL)
		return NULL;

	while(strrchr(*line,'\n') == NULL){
		*max_line_len *= 2;
		*line = (char *) realloc(*line,*max_line_len);
		len = (int) strlen(*line);
		if(fgets(*line+len,*max_line_len-len,input) == NULL)
			break;
	}
	return *line;
}

void read_problem(const char *filename, struct problem *prob, struct feature_node *x_space, double bias){
	int max_index, inst_max_index, i;
	int max_line_len;
	size_t elements, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;
	char *line = NULL;

	if(fp == NULL){
		fprintf(stderr,"can't open input file %s\n", filename);
		exit(1);
	}

	prob->l = 0;
	elements = 0;
	max_line_len = 1024;
	line = Malloc(char, max_line_len);
	while(readline(fp, &line, &max_line_len)!=NULL){
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
		readline(fp, &line, &max_line_len);
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
	free(line);
}

int main(int argc, char *argv[]){
	int neg_num;
	int pos_num;
	int svm_num;

	pos_num = atoi(argv[2]);
	neg_num = atoi(argv[3]);
	svm_num = pos_num * neg_num;

	struct feature_node *x_space = NULL;
	struct parameter param;
	struct problem prob;
	struct model* md[svm_num];
	struct timeval start, finish;

	char model_file_name[50];
	char train_file_name[50];
	char test_file_name[] = "../plain/svm_test";
	char mod[10];

	param.solver_type = L2R_L2LOSS_SVC_DUAL;
	param.C = 0.6;
	param.eps = 0.00001;
	param.p = 0.1;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	param.init_sol = NULL;
	double bias = -1;
	double count = 0;
	double timeuse;
	int index = 0;
	int result = 0;
	int flag_min;
	int flag_max;

	if(argc!=5){
		printUsage();
		return 0;
	}
	
	strcpy(mod, argv[1]);
	void (*print_func)(const char*) = &print_null;
	set_print_string_function(print_func);
	
	gettimeofday(&start, NULL);
	for(int i=0; i<neg_num; ++i){
		for(int j=0; j<pos_num; ++j){
			printf("Training model %d-%d-%d\n", index, i, j);
			sprintf(train_file_name, "../%s/data/%s_train_%d_%d_%d", mod, mod, index, i, j);
			sprintf(model_file_name, "../model/%s/%s_model_%d_%d_%d", mod, mod, index, i, j);
			read_problem(train_file_name, &prob, x_space, bias);
			md[index] = train(&prob, &param);
			save_model(model_file_name, md[index]);
			free(prob.y);
			free(prob.x);
			free(x_space);
			//free(line);
			++ index;
		}
	}
	gettimeofday(&finish, NULL);
	timeuse = (finish.tv_sec-start.tv_sec)+(finish.tv_usec-start.tv_usec)/1000000.0;
	printf("Time Used: %f\n", timeuse);

	read_problem(test_file_name, &prob, x_space, bias);
	printf("Num of data: %d\n", prob.l);

	if(!strcmp("grid", argv[4])){
	for(int p=0; p<neg_num; p+=1){
	for(int q=0; q<pos_num; q+=1){

	count = 0;
	int vote[neg_num][pos_num];
	for(int d=0; d<prob.l; ++d){
		index = 0;
		result = 0;
		for(int i=0; i<neg_num; ++i){
			for(int j=0; j<pos_num; ++j){
				vote[i][j] = int(predict(md[index], prob.x[d]));
				++ index;
			}
		}

		flag_max = 0;
		for(int i=0; i<pos_num; ++i){
			flag_min = 0;
			for(int j=0; j<neg_num; ++j)
				if(vote[j][i]) ++ flag_min;
			if(flag_min>p) ++ flag_max;
		}
		if(flag_max>q) result = 1;
		if(result==int(prob.y[d])) ++ count;
	}
	printf("%d, %d, %0.2f%%\n", p, q, 100*count/prob.l);

	}}}

	else{
	int min_th;
	int max_th;
	double predict;
	double decision;
	double tp[THRESHOD_NUM] = {0};
	double fp[THRESHOD_NUM] = {0};
	double tn[THRESHOD_NUM] = {0};
	double fn[THRESHOD_NUM] = {0};
	double th[] = {-8, -4, -2, -1, -0.5, 0, 0.5, 1, 2, 4, 8};
	double vote[neg_num][pos_num];
	if(!strcmp("rand", argv[1])) {min_th = RAND_MIN_TH; max_th = RAND_MAX_TH;}
	else {min_th = YC_MIN_TH; max_th = YC_MAX_TH;}
	for(int d=0; d<prob.l; ++d){
		index = 0;
		for(int i=0; i<neg_num; ++i){
			for(int j=0; j<pos_num; ++j){
				predict = int(predict_values(md[index], prob.x[d], &(vote[i][j])));
				++ index;
			}
		}

		for(int k=0; k<THRESHOD_NUM; ++k){
			result = 0;
			flag_max = 0;
			for(int i=0; i<pos_num; ++i){
				flag_min = 0;
				for(int j=0; j<neg_num; ++j)
					if(vote[j][i]<th[k]) ++ flag_min;
				if(flag_min>min_th) ++ flag_max;
			}
			if(flag_max>max_th) result = 1;
			if(prob.y[d]==1 && result==1) ++(tp[k]);
			else if(prob.y[d]==1 && result==0) ++(fn[k]);
			else if(prob.y[d]==0 && result==1) ++(fp[k]);
			else if(prob.y[d]==0 && result==0) ++(tn[k]);
		}
	}

	char outName[50];
	sprintf(outName, "m3_%s_test_data", argv[1]);
	FILE *out = fopen(outName, "w");
	for(int i=0; i<THRESHOD_NUM; ++i){
		printf("TH:%0.1f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f\n", th[i], tp[i]/prob.l, tn[i]/prob.l, fp[i]/prob.l, fn[i]/prob.l, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i]))));
		fprintf(out, "TH:%0.1f\tTP:%0.6f TN:%0.6f FP:%0.6f FN:%0.6f TPR:%0.4f FPR:%0.4f P:%0.4f R:%0.4f F1:%0.4f\n", th[i], tp[i]/prob.l, tn[i]/prob.l, fp[i]/prob.l, fn[i]/prob.l, tp[i]/(tp[i]+fn[i]), fp[i]/(fp[i]+tn[i]), tp[i]/(tp[i]+fp[i]), tp[i]/(tp[i]+fn[i]), 2*(tp[i]/(tp[i]+fn[i]))*(tp[i]/(tp[i]+fp[i]))/((tp[i]/(tp[i]+fn[i]))+(tp[i]/(tp[i]+fp[i]))));
	}
	fclose(out);
	}

	for(int i=0; i<svm_num; ++i) free_and_destroy_model(&md[i]);
	destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	//free(line);

	return 0;
}













