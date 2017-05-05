#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

struct node{
	int count;
	char cls[10];
	char name[50];
	struct node *next;
};

void swap(unsigned int *x, unsigned int *y){
	int temp = *x;
	*x = *y;
	*y = temp;
}

FILE *search(struct node *head, char *cls){
	if(!head->next) return NULL;

	struct node *p = head->next;
	while(p){
		if(p->cls[0]==cls[0] && p->cls[1]==cls[1] && p->cls[2]==cls[2]){
			p->count += 1;
			return fopen(p->name, "a");
		}
		else p = p->next;
	}
	return NULL;
}

FILE *find(struct node *head, int *min, char label){
	struct node *r = NULL;
	struct node *p = head->next;
	int minSize = *min;
	int maxSize = 2147483640;
	while(p){
		//printf("%c, %c, %d, %d, %d\n", label, p->cls[0], minSize, maxSize, p->count);
		if(label=='1' && p->cls[0]=='A' && p->count>minSize && p->count<maxSize){
			maxSize = p->count;
			r = p;
		}
		else if(label=='0' && p->cls[0]!='A' && p->count>minSize && p->count<maxSize){
			maxSize = p->count;
			r = p;
		}
		p = p->next;
	}
	if(r){
		*min = r->count;
		return fopen(r->name, "r");
	}
	else return NULL;
}

void addlist(struct node *head, char *cls, char *name){
	struct node *p = head;
	while(p->next) p = p->next;
	p->next = (struct node *)malloc(sizeof(int)+10*sizeof(int)+50*sizeof(char)+sizeof(struct node *));
	p = p->next;
	p->count = 1;
	p->cls[0] = cls[0];
	p->cls[1] = cls[1];
	p->cls[2] = cls[2];
	p->next = NULL;
	strcpy(p->name, name);
}

void destroy(struct node *head){
	struct node *p;
	head = head->next;
	while(head){
		p = head->next;
		free(head);
		head = p;
	}
}

void dataSvm(char *filename){
	unsigned int count = 0;
	int flag = 0;
	char temp[500];
	
	FILE *input = NULL;
	FILE *output = NULL;

	input = fopen(filename, "r");
	if(input==NULL){
		printf("File doesn't exists !\n");
		return;
	}
	strcpy(temp, "../plain/svm_");
	strcat(temp, filename);
	output = fopen(temp, "w");

	while(fscanf(input, "%s", temp)!=EOF){
		if(temp[0]=='A'){
			++ count;
			printf("\b\b\b\b\b\b%d", count);
			if(flag == 0){
				fprintf(output, "1 ");
				flag = 1;
			}
			else{
				fprintf(output, "\n1 ");
			}
		}
		else if(temp[0]>='B' && temp[0]<='D'){
			++ count;
			printf("\b\b\b\b\b\b%d", count);
			if(flag == 0){
				fprintf(output, "0 ");
				flag = 1;
			}
			else{
				fprintf(output, "\n0 ");
			}
		}
		else if(temp[0]>='0' && temp[0]<='9'){
			fprintf(output, "%s ", temp);
		}
		else{
			break;
		}
	}
	fclose(output);
	fclose(input);
}

void dataRand(char *filename, char *size){
	unsigned int PosFileCount = 0;
	unsigned int NegFileCount = 0;
	unsigned int PosFileMax = 0;
	unsigned int NegFileMax = 0;
	unsigned int PosDataCount = 0;
	unsigned int NegDataCount = 0;
	unsigned int PosDataSize = 0;
	unsigned int NegDataSize = 0;
	unsigned int total = 0;
	int flag = 0;
	int data_set_size = atoi(size);
	char temp[500];
	char name[50];
	char index[50];

	FILE *input = NULL;
	FILE *output_p = NULL;
	FILE *output_n = NULL;

	input = fopen(filename, "r");
	if(input == NULL){
		printf("File doesn't exists !\n");
		return;
	}
	while(fscanf(input, "%s", temp)!=EOF){
		if(temp[0]=='A') ++ PosDataCount;
		else if(temp[0]>='B' && temp[0]<='D') ++ NegDataCount;
	}
	PosFileMax = (int)round((double)PosDataCount / data_set_size);
	PosDataSize = (int)round((double)PosDataCount / PosFileMax);
	NegFileMax = (int)round((double)NegDataCount / data_set_size);
	NegDataSize = (int)round((double)NegDataCount / NegFileMax);
	printf("\nPosData:%d NegData:%d\n", PosDataCount, NegDataCount);
	//printf("%d, %d\n", PosDataSize, NegDataSize);
	//printf("%d, %d\n", PosFileMax, NegFileMax);

	PosDataCount = 0;
	NegDataCount = 0;

	strcpy(name, "../rand/rand_");
	strcat(name, filename);
	sprintf(index, "_p_%d", PosFileCount);
	strcat(name, index);
	output_p = fopen(name, "w");
	if(output_p == NULL){
		printf("File can't be created !\n");
		return;
	}

	strcpy(name, "../rand/rand_");
	strcat(name, filename);
	sprintf(index, "_n_%d", NegFileCount);
	strcat(name, index);
	output_n = fopen(name, "w");
	if(output_n == NULL){
		printf("File can't be created !\n");
		return;
	}

	printf("Start Processing ...\n");

	fseek(input, 0, SEEK_SET);
	while(fscanf(input, "%s", temp)!=EOF){
		printf("\b\b\b\b\b\b%d", total);
		if(temp[0]=='A'){
			if(PosDataCount == PosDataSize){
				++ PosFileCount;
				flag = 0;
				PosDataCount = 0;
				fclose(output_p);
				strcpy(name, "../rand/rand_");
				strcat(name, filename);
				sprintf(index, "_p_%d", PosFileCount);
				strcat(name, index);
				output_p = fopen(name, "w");
				if(output_p == NULL){
					printf("File can't be created !\n");
					return;
				}
			}
			if(flag == 0 || PosDataCount == 0) fprintf(output_p, "1 ");
			else fprintf(output_p, "\n1 ");
			flag = 1;
			++ total;
			++ PosDataCount;
		}
		else if(temp[0]>='B' && temp[0]<='D'){
			if(NegDataCount == NegDataSize){
				++ NegFileCount;
				flag = 0;
				NegDataCount = 0;
				fclose(output_n);
				strcpy(name, "../rand/rand_");
				strcat(name, filename);
				sprintf(index, "_n_%d", NegFileCount);
				strcat(name, index);
				output_n = fopen(name, "w");
				if(output_n == NULL){
					printf("File can't be created !\n");
					return;
				}
			}
			if(flag == 0 || NegDataCount == 0) fprintf(output_n, "0 ");
			else fprintf(output_n, "\n0 ");
			flag = -1;
			++ total;
			++ NegDataCount;
		}
		else if(flag==1){
			fprintf(output_p, "%s ", temp);
		}
		else{
			fprintf(output_n, "%s ", temp);
		}
	}

	fclose(input);
	fclose(output_p);
	fclose(output_n);

	printf("\nCombining ...\n");
	FILE *pos;
	FILE *neg;
	FILE *out;
	int count = 0;
	int trunk = 0;

	for(int i=0; i<=NegFileCount; ++i){
		strcpy(name, "../rand/rand_");
		strcat(name, filename);
		sprintf(index, "_n_%d", i);
		strcat(name, index);
		neg = fopen(name, "r");
		for(int j=0; j<=PosFileCount; ++j){
			strcpy(name, "../rand/rand_");
			strcat(name, filename);
			sprintf(index, "_p_%d", j);
			strcat(name, index);
			pos = fopen(name, "r");

			strcpy(name, "../rand/data/rand_");
			strcat(name, filename);
			sprintf(index, "_%d_%d_%d", count, i, j);
			strcat(name, index);
			out = fopen(name, "w");

			while(trunk = (int)fread(temp, 1, 500, neg)) fwrite(temp, 1, trunk, out);
			fprintf(out, "\n");
			while(trunk = (int)fread(temp, 1, 500, pos)) fwrite(temp, 1, trunk, out);
			
			fseek(neg, 0, SEEK_SET);
			fclose(pos);
			fclose(out);
			++ count;
		}
		fclose(neg);
	}
	printf("PosNum:%d NegNum:%d\n", PosFileCount+1, NegFileCount+1);
}

void dataYc(char *filename, char *size){
	unsigned int count = 0;
	unsigned int posCount = 0;
	unsigned int negCount = 0;
	int data_set_size = atoi(size);

	char temp[500];
	char name[50];
	char index[50];

	FILE *input = NULL;
	FILE *output = NULL;
	
	struct node list;
	list.next = NULL;

	input = fopen(filename, "r");
	if(input == NULL){
		printf("File doesn't exists !\n");
		return;
	}
	
	while(fscanf(input, "%s", temp)!=EOF){
		if(temp[0]>='A' && temp[0]<='D'){
			++ count;
			printf("\b\b\b\b\b\b%d", count);
			if(output) fclose(output);
			output = search(&list, temp);
			if(!output){
				sprintf(name, "../yc/yc");
				//strcat(name, filename);
				if(temp[0] == 'A'){
					strcat(name, "_p_");
					sprintf(index, "%d", posCount);
					strcat(name, index);
					++ posCount;
				}
				else{
					strcat(name, "_n_");
					sprintf(index, "%d", negCount);
					strcat(name, index);
					++ negCount;
				}
				output = fopen(name, "w");
				if(output == NULL){
					printf("File can't be opened !\n");
					return;
				}
				addlist(&list, temp, name);
				if(temp[0]=='A') fprintf(output, "1 ");
				else fprintf(output, "0 ");
			}
			else{
				if(temp[0]=='A') fprintf(output, "\n1 ");
				else fprintf(output, "\n0 ");
			}
		}
		else fprintf(output, "%s ", temp);
	}

	fclose(input);
	fclose(output);
	//destroy(&list);

	printf("\nPosCls:%d NegCls:%d\nCombining ...\n", posCount, negCount);
	FILE *pos;
	FILE *neg;
	FILE *out;
	int dataCount = 1;
	int posFileCount = 0;
	int negFileCount = 0;
	int posMinSize = 0;
	int negMinSize = 0;

	pos = find(&list, &posMinSize, '1');
	sprintf(name, "../yc/yc_%s_p_%d", filename, posFileCount);
	//printf("filename:%s\n", name);
	out = fopen(name, "w");
	fscanf(pos, "%s", temp);
	fprintf(out, "%s ", temp);
	while(1){
		if(!pos){
			fclose(out);
			break;
		}
		else{
			if(fscanf(pos, "%s", temp)==EOF){
				fclose(pos);
				pos = find(&list, &posMinSize, '1');
				if(pos){
					fscanf(pos, "%s", temp);
					if(dataCount>data_set_size){
						++ posFileCount;
						dataCount = 0;
						fclose(out);
						sprintf(name, "../yc/yc_%s_p_%d", filename, posFileCount);
						//printf("filename:%s\n", name);
						out = fopen(name, "w");
						fprintf(out, "%s ", temp);
						++ dataCount;
					}
					else{
						++ dataCount;
						fprintf(out, "\n%s ", temp);
					}
				}
			}
			else{
				if(!strcmp(temp, "1")){
					if(dataCount>data_set_size){
						++ posFileCount;
						dataCount = 0;
						fclose(out);
						sprintf(name, "../yc/yc_%s_p_%d", filename, posFileCount);
						//printf("filename:%s\n", name);
						out = fopen(name, "w");
						fprintf(out, "%s ", temp);
						++ dataCount;
					}
					else{
						fprintf(out, "\n%s ", temp);
						++ dataCount;
					}
				}
				else fprintf(out, "%s ", temp);
			}
		}
		
	}

	dataCount = 1;
	neg = find(&list, &negMinSize, '0');
	sprintf(name, "../yc/yc_%s_n_%d", filename, negFileCount);
	//printf("filename:%s\n", name);
	out = fopen(name, "w");
	fscanf(neg, "%s", temp);
	fprintf(out, "%s ", temp);
	while(1){
		if(!neg){
			fclose(out);
			break;
		}
		else{
			if(fscanf(neg, "%s", temp)==EOF){
				fclose(neg);
				neg = find(&list, &negMinSize, '0');
				if(neg){
					fscanf(neg, "%s", temp);
					if(dataCount>data_set_size){
						++ negFileCount;
						dataCount = 0;
						fclose(out);
						sprintf(name, "../yc/yc_%s_n_%d", filename, negFileCount);
						//printf("filename:%s\n", name);
						out = fopen(name, "w");
						fprintf(out, "%s ", temp);
						++ dataCount;
					}
					else{
						++ dataCount;
						fprintf(out, "\n%s ", temp);
					}
				}
			}
			else{
				if(!strcmp(temp, "0")){
					if(dataCount>data_set_size){
						++ negFileCount;
						dataCount = 0;
						fclose(out);
						sprintf(name, "../yc/yc_%s_n_%d", filename, negFileCount);
						//printf("filename:%s\n", name);
						out = fopen(name, "w");
						fprintf(out, "%s ", temp);
						++ dataCount;
					}
					else{
						fprintf(out, "\n%s ", temp);
						++ dataCount;
					}
				}
				else fprintf(out, "%s ", temp);
			}
		}
		
	}
	printf("PosNum:%d NegNum:%d\n", posFileCount+1, negFileCount+1);		
	destroy(&list);	
	int trunk = 0;
	count = 0;

	for(int i=0; i<=negFileCount; ++i){
		strcpy(name, "../yc/yc_");
		strcat(name, filename);
		sprintf(index, "_n_%d", i);
		strcat(name, index);
		//printf("%s\n", name);
		neg = fopen(name, "r");
		if(!neg) printf("Neg Open Failed\n");
		for(int j=0; j<=posFileCount; ++j){
			strcpy(name, "../yc/yc_");
			strcat(name, filename);
			sprintf(index, "_p_%d", j);
			strcat(name, index);
			//printf("%s\n", name);
			pos = fopen(name, "r");
			if(!pos) printf("Pos Open Failed\n");

			strcpy(name, "../yc/data/yc_");
			strcat(name, filename);
			sprintf(index, "_%d_%d_%d", count, i, j);
			strcat(name, index);
			//printf("%s\n", name);
			out = fopen(name, "w");
			if(!out) printf("Out Open Failed\n");

			//printf("Start writing ...\n");
			while(trunk = (int)fread(temp, 1, 500, neg)) fwrite(temp, 1, trunk, out);
			fprintf(out, "\n");
			while(trunk = (int)fread(temp, 1, 500, pos)) fwrite(temp, 1, trunk, out);
			//printf("Stop writing ...\n");

			fseek(neg, 0, SEEK_SET);
			fclose(pos);
			fclose(out);
			++ count;
			//printf("%d\n", count);
		}
		fclose(neg);
	}
}

void showUsage(){
	printf("Usage: data [options] [filename] [class size]\n");
	printf("Options:\n");
	printf("\t0\tcreate data files for svm\n");
	printf("\t1\tCreate data files for m3-rand\n");
	printf("\t2\tCreate data files for m3-yc\n");
	return;
}

int main(int argc, char *argv[]){
	if(argc==1){
		showUsage();
		return 0;
	}
	if(!strcmp(argv[1], "0")){
		printf("Preparing data for svm ...\n");
		dataSvm(argv[2]);
		printf("\nDone !\n");
	}
	else if(!strcmp(argv[1], "1")){
		printf("Preparing data for m3-rand ...\n");
		dataRand(argv[2], argv[3]);
		printf("\nDone !\n");
	}
	else if(!strcmp(argv[1], "2")){
		printf("Preparing data for m3-yc ...\n");
		dataYc(argv[2], argv[3]);
		printf("\nDone !\n");
	}
	else{
		printf("Oops ! Wrong option !\n");
		showUsage();
	}
	return 0;
}
