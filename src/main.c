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

#include <stdarg.h>

#define FTPDIR_w "D:\\data\\ftp"
#define FTPDIR "/home/glr/src/example/"
#define LOGDIR "/tmp/"

#ifndef _WIN32
#define TMPTEMP "/tmp/sdelionXXXXXX"
#endif

// ./sde.exe /home/glr/src/example/ /home/glr/tmp
int check_tar(char ** sdepoint, FILE * log);
int myread_tar(char ** sdepoint, FILE * log, char ** tmppath);


int sprintf_alloc(char **str, const char *fmt, ...)
{
    va_list ap;
    char *new_str;
    int n, size = 100;

    if (!str) {
      fprintf(stderr, "Null string pointer passed to sprintf_alloc\n");
      return -1;
    }
    if (!fmt) {
      fprintf(stderr, "Null fmt string passed to sprintf_alloc\n");
      return -1;
    }

    /* On x86_64 systems, any strings over 100 were segfaulting.  
       It seems that the ap needs to be reinitalized before every
       use of the v*printf() functions. I pulled the functionality out
       of vsprintf_alloc and combined it all here instead. 
    */
    

    /* ripped more or less straight out of PRINTF(3) */

    if ((new_str = calloc(1, size)) == NULL) 
      return -1;

    *str = new_str;
    while(1) {
      va_start(ap, fmt);
      n = vsnprintf (new_str, size, fmt, ap);
      va_end(ap);
      /* If that worked, return the size. */
      if (n > -1 && n < size)
    return n;
    /* Else try again with more space. */
    if (n > -1)    /* glibc 2.1 */
        size = n+1; /* precisely what is needed */
    else           /* glibc 2.0 */
        size *= 2;  /* twice the old size */
    new_str = realloc(new_str, size);
    if (new_str == NULL) {
        free(new_str);
        *str = NULL;
        return -1;
    }
    *str = new_str;
    }

    return -1; /* Just to be correct - it probably won't get here */
}

int main(int argc,char *argv[])
{
    DIR *dp;
	FILE *log;
    


    struct dirent * dir_entry;
    struct stat statbuf;    
    int r=0;
    int exitcode=0;
    char *sdedir=malloc(100);
    char *sdepath=malloc(120);
	char *logpath=malloc(120);

    if (argc > 1)
    {
        strcpy(sdedir,argv[1]);
        if (argc >2 ){
            strcpy(logpath,argv[2]);
        } else {
            strcpy(logpath,LOGDIR);
        }

    } else {
        strcpy(logpath,LOGDIR);
        strcpy(sdedir,FTPDIR);
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

    if ((dp=opendir(sdedir))==NULL) {
		fprintf(stderr,"can not open dir %s", sdedir);
		exit(0);
	}
    chdir(sdedir);
	printf("SDE DIR %s \n", sdedir);
	log=fopen(logpath,"w");
    printf("LOG DIR %s \n",logpath);

    #ifdef _WIN32
    //fs->font_directory = "c:/fontfiles";    //FIX ME
    char *tmpdir = strdup(TMPTEMP);
    mkdtemp(tmpdir);
    #else
    char *tmpdir = strdup(TMPTEMP);
    mkdtemp(tmpdir);
    printf("tmpdir: %s\n", tmpdir);
    #endif




    while ((dir_entry=readdir(dp)) != NULL) {
		stat(dir_entry->d_name,&statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			printf("FOLDER %s\n", dir_entry->d_name);
		} else {
            memset(sdepath,0,60);
            strcpy(sdepath,sdedir);
            strcat(sdepath,"/");
            strcat(sdepath,dir_entry->d_name);
            r=myread_tar( &sdepath, log,&tmpdir);
            
		}
	}
	fclose(log);
    //rmdir(tmpdir);

 
    if (r != ARCHIVE_OK)
        exitcode=1;
    closedir(dp);
    exit(exitcode);
}

int myread_tar(char ** sdepoint, FILE * log, char **tmppath)
{
    int r, len;
    int compo_count=0;
    int list_count=0;
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    char *tmpsdedirpath;
    
    int buffsize;
    //char *buff;
    char *sdename="sde";
    int flags = ARCHIVE_EXTRACT_TIME;
                flags |= ARCHIVE_EXTRACT_PERM;
                flags |= ARCHIVE_EXTRACT_ACL;
                flags |= ARCHIVE_EXTRACT_FFLAGS;

    int r2;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif


    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_read_support_format_tar(a);
    char *sdepath=* sdepoint;
    printf("%s\n",sdepath);
    r = archive_read_open_filename(a, sdepath, 10240);
    if (r != ARCHIVE_OK)
    {       
        printf("why archive not ok\n");
    } else {

                sprintf_alloc(&tmpsdedirpath, "%s/%s", *tmppath,sdename);

                printf("tmpsdedirpath %s\n",tmpsdedirpath);
                mkdir(tmpsdedirpath,01775);
                chdir(tmpsdedirpath);

        compo_count=0;
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            r = archive_write_header(ext, entry);
            if (list_count == 0 ){
                printf("%s\n",archive_entry_pathname(entry));
            }
            compo_count++;
            if ((buffsize=archive_entry_size(entry) )> 0) {
                //sdename=get_name(sdepath);
                

                //buff=malloc(buffsize);
            
                for (;;) {
                    r2 = archive_read_data_block(a, &buff, &size, &offset);
                    if (r2 == ARCHIVE_EOF){
                        break;
                    }
                    if (r2 != ARCHIVE_OK)
                        break;
                    r2 = archive_write_data_block(ext, buff, size, offset);
                    if (r2 != ARCHIVE_OK) {
                        //warn("archive_write_data_block()", archive_error_string(aw));
                        break;
                    }
                }
                r = archive_write_finish_entry(ext);
                //free(buff);
            } 
            //archive_read_data_skip(a);  
            //fprintf(log,"%s %d \n",archive_entry_pathname(entry),buffsize);
        }
        if ((compo_count != 12) & (compo_count != 11) ) {
            list_count++;
            printf("compo_count %d ", compo_count);
                        printf("File %s\n",sdepath);
            //getchar();
        }

        fprintf(log,"%d\n\n\n",compo_count);

    }
    r = archive_read_close(a); 

    r = archive_read_free(a); 
    return r;
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