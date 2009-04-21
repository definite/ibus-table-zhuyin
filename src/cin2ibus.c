#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define USAGE_MSG \
    "Usage: %s [-h] [-V] [-o <outFile>] <tmplFile> <cinFile> \n"\
    "Convert XCIN cin file (input table) to IBus table template.txt\n"\
    "-h: help\n"\
    "-V: verbose\n"\
    "-o outFile: output file, if this option is not given, then output to stdout\n"\
    "tmplFile: ibus-table template file, example can be found in %s/tables/template.txt\n\n"\
    "cinFile: input table file.\n\n"\
    "Note: MAX_KEY_LENGTH, VALID_INPUT_CHARS in template will be replaced.\n"


bool verbose=false;
#define VERBOSE_PRINT(msg,args...) if (verbose) printf(msg,##args);

const char *cinFile_name=NULL;
FILE *cinFile=NULL;
const char *tmplFile_name=NULL;
FILE *tmplFile=NULL;
const char *outFile_name=NULL;
FILE *outFile=NULL;
char cinTableFile_name[]="/tmp/cin.table.XXXXXX";
FILE *cinTableFile=NULL; // temporary file
char cinGouCiFile_name[]="/tmp/cin.GouCi.XXXXXX";
FILE *cinGouCiFile=NULL; // temporary file

int maxKeyLength=0;
int validInputChars_len=0;
#define VALID_INPUT_CHARS_BUFFER_SIZE 200
char validInputChars[VALID_INPUT_CHARS_BUFFER_SIZE];

static void usage_print(const char *programName){
    printf( USAGE_MSG, PROJECT_DATADIR, programName);
}

typedef enum{
    SCAN_STAGE_INIT,
    SCAN_STAGE_KEYNAME,
    SCAN_STAGE_CHARDEF
} ScanStage;

typedef struct{
    int num;
    char *field[4];
} Fields;

static void fields_parse(char *buf, Fields *fields){
    int i;
    bool ended=false;
    for (i=0;i<4;i++){
	if (ended){
	    fields->field[i]=NULL;
	}
	if (i==0){
	    fields->field[i]=strtok(buf," \t\n\r");
	}else{
	    fields->field[i]=strtok(NULL," \t\n\r");
	}
	if (!fields->field[i]){
	    ended=true;
	    fields->num=i;
	}
    }
    if (!ended){
	fields->num=i;
    }
}

#define FILE_READ_BUFFER_SIZE 500
#define FREQ_INIT 1000
static bool cinFile_scan(){
    VERBOSE_PRINT("*** Scanning cinFile %s\n",cinFile_name);
    ScanStage stage=SCAN_STAGE_INIT;
    char buf[FILE_READ_BUFFER_SIZE];
    Fields fields;
    int freq;
    bool convert_keyname=false;
    char key_last[FILE_READ_BUFFER_SIZE];
    key_last[0]='\0';

    while(fgets(buf,FILE_READ_BUFFER_SIZE, cinFile)!=NULL){
	VERBOSE_PRINT("stage=%d buf=%s|\n",stage,buf);
	fields_parse(buf,&fields);
	switch(stage){
	    case SCAN_STAGE_INIT:
		if ((strcmp(fields.field[0],"%keyname")==0) && (strcmp(fields.field[1],"begin")==0) ){
		    stage=SCAN_STAGE_KEYNAME;
		    VERBOSE_PRINT("Reading %%keyname");
		    convert_keyname=true;
		}else if ((strcmp(fields.field[0],"%chardef")==0) && (strcmp(fields.field[1],"begin")==0) ){
		    stage=SCAN_STAGE_CHARDEF;
		    VERBOSE_PRINT("Reading %%chardef");
		}
		break;
	    case SCAN_STAGE_KEYNAME:
		if ((strcmp(fields.field[0],"%keyname")==0) && (strcmp(fields.field[1],"end")==0) ){
		    VERBOSE_PRINT("Reading %%keyname Done");
		    break;
		}else if ((strcmp(fields.field[0],"%chardef")==0) && (strcmp(fields.field[1],"begin")==0) ){
		    VERBOSE_PRINT("Reading %%chardef");
		    stage=SCAN_STAGE_CHARDEF;
		    break;
		}
		validInputChars[validInputChars_len++]=fields.field[0][0];
		validInputChars[validInputChars_len]='\0';
		fprintf(cinGouCiFile,"%s\t%s\n",fields.field[1],fields.field[0]);
		break;
	    case SCAN_STAGE_CHARDEF:
		if ((strcmp(fields.field[0],"%chardef")==0) && (strcmp(fields.field[1],"end")==0) ){
		    VERBOSE_PRINT("Reading %%chardef Done");
		    break;
		}
		if (strcmp(key_last,fields.field[0])==0){
		    freq-=5;
		}else{
		    freq=FREQ_INIT;
		    strcpy(key_last,fields.field[0]);
		    int key_len=strlen(fields.field[0]);
		    if (key_len > maxKeyLength){
			maxKeyLength=key_len;
		    }
		}
		fprintf(cinTableFile,"%s\t%s\t%d\n",fields.field[0],fields.field[1],freq);
		break;
	}
    }
    fflush(cinTableFile);
    fflush(cinGouCiFile);
    return true;
}

static bool file_cat(FILE *dest, FILE *src){
    char buf[FILE_READ_BUFFER_SIZE];
    while(fgets(buf,FILE_READ_BUFFER_SIZE, src)!=NULL){
	fputs(buf,dest);
    }
    return true;
}

static bool files_merge(){
    rewind(cinTableFile);
    rewind(cinGouCiFile);
    char buf[FILE_READ_BUFFER_SIZE];

    VERBOSE_PRINT("Merging files.\n");
    while(fgets(buf,FILE_READ_BUFFER_SIZE, tmplFile)!=NULL){
        bool printAsIs=true;
	if (strncasecmp(buf,"END_TABLE",strlen("END_TABLE"))==0){
	    file_cat(outFile,cinTableFile);
	}else if (strncasecmp(buf,"END_GOUCI",strlen("END_GOUCI"))==0){
	    file_cat(outFile,cinGouCiFile);
	}else if (strncmp(buf,"VALID_INPUT_CHARS",strlen("VALID_INPUT_CHARS"))==0){
	    fprintf(outFile,"VALID_INPUT_CHARS = %s\n",validInputChars);
	    printAsIs=false;
	}else if (strncmp(buf,"MAX_KEY_LENGTH",strlen("MAX_KEY_LENGTH"))==0){
	    fprintf(outFile,"MAX_KEY_LENGTH = %d\n",maxKeyLength);
	    printAsIs=false;
	}
	if (printAsIs){
	    fputs(buf,outFile);
	}
    }

    fclose(cinTableFile);
    fclose(cinGouCiFile);
    return true;
}

bool argument_is_valid(int argc, char * const argv[]){
    int opt;
    while((opt=getopt(argc,argv,"hVo:"))!=-1){
	switch (opt){
	    case 'h':
		usage_print(argv[0]);
		exit(0);
	    case 'V':
		verbose=true;
		break;
	    case 'o':
		if ((outFile=fopen(optarg,"w"))==NULL){
		    fprintf(stderr,"Cannot open %s for writing!\n",optarg);
		    exit(EXIT_FAILURE);
		}
		break;
	    default: /* '?' */
		usage_print(argv[0]);
		exit(EXIT_FAILURE);
	}
    }
    if (optind+2-1 > argc) {
	fprintf(stderr, "Not enough arguments.\n");
	usage_print(argv[0]);
	exit(-1);
    }

    tmplFile_name=argv[optind];
    if ((tmplFile=fopen(tmplFile_name,"r"))==NULL){
	fprintf(stderr,"Cannot open %s for reading!\n",tmplFile_name);
	exit(-1);
    }

    cinFile_name=argv[optind+1];
    if ((cinFile=fopen(cinFile_name,"r"))==NULL){
	fprintf(stderr,"Cannot open %s for reading!\n",cinFile_name);
	exit(-1);
    }

    if ((cinTableFile=fdopen(mkstemp(cinTableFile_name),"w+"))==NULL){
	fprintf(stderr,"Cannot create temporary file %s for reading!\n",cinTableFile_name);
	exit(-2);
    }

    if ((cinGouCiFile=fdopen(mkstemp(cinGouCiFile_name),"w+"))==NULL){
	fprintf(stderr,"Cannot create temporary file %s for reading!\n",cinGouCiFile_name);
	exit(-2);
    }

    if (!outFile){
	outFile=stdout;
    }
    return true;
}
#undef FILE_READ_BUFFER_SIZE

int main(int argc, char * const argv[]){
    if (!argument_is_valid(argc,argv)){
	usage_print(argv[0]);
	return -1;
    }

    if (!cinFile_scan()){
	fprintf(stderr,"Failed when reading %s!\n",cinFile_name);
	return 1;
    }

    if (!files_merge()){
	fprintf(stderr,"Failed when merging files!\n");
	return 3;
    }

    fclose(cinFile);
    fclose(tmplFile);
    if (outFile!=stdout){
	fclose(outFile);
    }

    return 0;
}
