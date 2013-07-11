#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <archive.h>
#include <archive_entry.h>
#include <dirent.h>

#define FTPDIR "D:\\data\\ftp"
// ./sde.exe /home/glr/src/example/ /home/glr/tmp
int check_tar(char ** sdepoint, FILE * log);



int main(int argc,char *argv[])
{
    DIR *dp;
	FILE *log;
    
    char *path;


    struct dirent * dir_entry;
    struct stat statbuf;    
    int r=0;
    int exitcode=0;
    char *sdepath=malloc(120);
	char *logpath=malloc(120);


	

    if (argc > 1)
    {
        path=argv[1];
    }
    
    if (argc >2 ){
        strcpy(logpath,argv[2]);
        
    }
    
          char outstr[200];
           time_t t;
           struct tm *tmp;

           t = time(NULL);
           tmp = localtime(&t);
           if (tmp == NULL) {
               perror("localtime");
               exit(EXIT_FAILURE);
           }

           if (strftime(outstr, sizeof(outstr),"%Y%m%dT%H%M%Slog.log" , tmp) == 0) {
               fprintf(stderr, "strftime returned 0");
               exit(EXIT_FAILURE);
           }

 
    
    strcat(logpath,outstr);

    //archive_read_support_filter_all(a);

    if ((dp=opendir(path))==NULL) {
		fprintf(stderr,"can not open dir %s", path);
		exit(0);
	}
    chdir(path);
	printf("%s \n", path);
	log=fopen(logpath,"w");
    printf("%s \n",logpath);

    while ((dir_entry=readdir(dp)) != NULL) {
		stat(dir_entry->d_name,&statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			printf("FOLDER %s\n", dir_entry->d_name);
		} else {
            memset(sdepath,0,60);
            strcpy(sdepath,path);
            strcat(sdepath,"/");
            strcat(sdepath,dir_entry->d_name);
            r=check_tar( &sdepath, log);
            
		}
	}
	fclose(log);


 
    if (r != ARCHIVE_OK)
        exitcode=1;
    closedir(dp);
    exit(exitcode);
}

int check_tar(char ** sdepoint, FILE * log)
{
    int r, len;
    int compo_count=0;
    int list_count=0;
    struct archive *a;
    struct archive_entry *entry;
    
    int buffsize;
    char *buff;
            
            a = archive_read_new();
            archive_read_support_format_tar(a);
            char *sdepath=* sdepoint;
            printf("%s\n",sdepath);
            r = archive_read_open_filename(a, sdepath, 10240);
            if (r != ARCHIVE_OK)
            {       
		    printf("why archive not ok\n");
		    ;
        
            } else {
                compo_count=0;
                while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
                    printf("%d:\n", __LINE__);
                    if (list_count == 0 ){
                        printf("%s\n",archive_entry_pathname(entry));
                    }
                    compo_count++;
                    if ((buffsize=archive_entry_size(entry) )> 0) {
                        buff=malloc(buffsize);
                        printf("%d:\n", __LINE__);
                        
                        
                        len = archive_read_data(a, buff, buffsize);
                        if (len != buffsize) {
                            printf("Len %d --- buff %d",len, buffsize);
                        } else {
                            printf(".");
                        }
                        while (len > 0) {
                            

                                len = archive_read_data(a, buff, sizeof(buff));
                        }
                        if (len < 0){
                            printf("Error reading input archive");
                            return(-1);
                        }
                        free(buff);
                    } 
                    printf("%d:\n", __LINE__);
                    //archive_read_data_skip(a);  
					//fprintf(log,"%s %d \n",archive_entry_pathname(entry),buffsize);

					printf("%d:\n", __LINE__);
                }
                printf("%d:\n", __LINE__);
                if ((compo_count != 12) & (compo_count != 11) ) {
                    list_count++;
                    printf("compo_count %d ", compo_count);
								printf("File %s\n",sdepath);
                    //getchar();
                }
                printf("%d:\n", __LINE__);
					fprintf(log,"%d\n\n\n",compo_count);
            }
            printf("%d:\n", __LINE__);
            r = archive_read_free(a); 
            printf("%d:\n", __LINE__);
            return r;
}