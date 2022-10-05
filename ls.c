//Used for basic input/output stream
#include <stdio.h>
//Used for handling directory files
#include <dirent.h>

//For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>

// Other dependencies
#include <string.h>
#include <sys/stat.h>
// #include <sys/acl.h>
// #include <acl/libacl.h>
#include "include/file_perms.h"
#include <time.h>
#include <pwd.h>
#include <grp.h>

#define MAX_SIZE 255

//Constant definition
// const int perm[3]={ACL_READ,ACL_WRITE, ACL_EXECUTE};
// const char perm_char[3]= {'r','w','x'};
// const int file_perms[3]={ACL_USER_OBJ,ACL_GROUP_OBJ,ACL_OTHER};

// int is_contained(const int* arr, int value){
//   int size = sizeof(arr)/sizeof(int)+1;

//   for(int i =0; i<size; i++){
//     if(value == arr[i])
//       return i;
//   }

//   return -1;
// }

// char* conv_tag_i2c(int tag){
//     switch(tag){
//       case(ACL_UNDEFINED_TAG):
//         return "Undefine";
//       case(ACL_USER_OBJ):
//         return "Owner";
//       case(ACL_USER):
//         return "User"
//       #define ACL_USER		(0x02)
// #define ACL_GROUP_OBJ		(0x04)
// #define ACL_GROUP		(0x08)
// #define ACL_MASK		(0x10)
// #define ACL_OTHER		(0x20)
//     }

// }



//*************Permission set operation***************
// char* convert_perm_i2c(int* perm){
//   char * cperm = (char*)malloc(3*sizeof(char));
//   for(int i=0; i<3 ; i++){
//     if(perm[i]!=1){
//       cperm[i]= '-';
//       continue;
//     }
//     cperm[i] = perm_char[i];
//   }
//   return cperm;
// }

// char* get_entry_perm(acl_entry_t entry){
//   //---Get entry permission set
//   acl_permset_t perm_set;
//   int status;

//   if((status = acl_get_permset(entry, &perm_set))==-1){
//     return NULL;
//   }

//   int* iperm_set = (int*)malloc(3*sizeof(int));
//   for(int i=0; i<3; i++){
//     iperm_set[i]= acl_get_perm(perm_set,perm[i]);
//   }

//   //-----Convert to character permisson set
//   char * cperm_set = convert_perm_i2c(iperm_set);
//   free(iperm_set);
//   return cperm_set;
// }


// char* get_file_perms(char* path){

//   acl_t acl;
//   acl = acl_get_file(path,ACL_TYPE_ACCESS);

//   acl_entry_t entry;
//   int status = acl_get_entry(acl,ACL_FIRST_ENTRY, &entry);

//   if (status != 1){
//     printf("Error when try to access ACL entry");
//     return NULL;
//   }

//   //Get file permission sets
//   char* result = (char*)malloc(9*sizeof(char));

//   acl_tag_t tag_type;
//   status = acl_get_tag_type(entry, &tag_type);
//   if (status == -1){
//     printf("Error when try to get tag type of ACL entry");
//     return NULL;
//   }
  
//   int index = is_contained(file_perms, tag_type);
//   if(index != -1){
//     char* cperm_set = get_entry_perm(entry);
//     if (cperm_set == NULL){
//       return NULL;
//     }
//     memcpy(result+index*3,cperm_set,3);
//     free(cperm_set);
//   }

//   while((status = acl_get_entry(acl,ACL_NEXT_ENTRY, &entry))!=0){
//     if (status != 1){
//       return NULL;
//     }

//     status = acl_get_tag_type(entry, &tag_type);
//     if (status == -1){
//       return NULL;
//     }

//     int index = is_contained(file_perms, tag_type);
//     if(index != -1){
//       char* cperm_set = get_entry_perm(entry);
//       if (cperm_set == NULL){
//         return NULL;
//       }
//       memcpy(result+index*3,cperm_set,3);
//       free(cperm_set);
//     }
//   }

//   return result;
// }

char get_file_type(mode_t st_mode){
  char type;
  switch(st_mode & S_IFMT){
    case S_IFREG:
      type='-';
      break;
    case S_IFDIR:
      type='d';
      break;
    case S_IFCHR:
      type='c';
      break;
    case S_IFBLK:
      type='b';
      break;
    case S_IFLNK:
      type='l';
      break;
    case S_IFIFO:
      type='p';
      break;
    case S_IFSOCK:
      type='s';
      break;
    default:
      type=' ';
      break;
  }

  return type;
}

char* file_info(const char * path){
  struct stat stat_ds;

  int status = stat(path,&stat_ds);

  // Get file type
  char type = get_file_type(stat_ds.st_mode);

  // Get file permissions
  char * perms = filePermStr(stat_ds.st_mode, 0);

  // Get file hardlinks
  long nlink = (long)stat_ds.st_nlink;

  // Get file user & group name
  char * user_name = getpwuid(stat_ds.st_uid)->pw_name;
  char * group_name = getgrgid(stat_ds.st_gid)->gr_name;
 
  // Get file size
  long long size = (long long)stat_ds.st_size;

  // Get file last access
  char * last_access = ctime(&stat_ds.st_mtim.tv_sec);
  int  last_access_size =  strlen(last_access);
  char *m_last_access = (char*)malloc(sizeof(char)*last_access_size-sizeof(char));
  memcpy(m_last_access, last_access,last_access_size-1);
  
  
  char * result = (char*)malloc(MAX_SIZE);
  snprintf(result,MAX_SIZE ,"%c%s %ld %s %s %lld %s" ,type,perms,nlink, user_name,group_name,size,m_last_access);

  return result;
}

void _ls(const char *dir, int op_a ,int op_l){
    struct dirent *d;
    DIR *dh = opendir(dir);


    // Check if there are directory
    if(!dh){
      if(errno== ENOENT){
        perror("Can not find the directory");
      }else{
        perror("Invalid referenced pointer");
      }
      exit(EXIT_FAILURE);
    }
    

    // If ok, loop through the stream and print each file
    while((d=readdir(dh)) != NULL){
      //  if op_a print all of file
      if(!op_a && d->d_name[0]=='.')
        continue;

      if(op_l){
        printf("%s %s\n", file_info(d->d_name), d->d_name);
        continue; 
      }

      printf("%s ", d->d_name);
    }
}



int main(int argc, const char* argv[]){
    if(argc == 1){
      _ls(".",0,0);
    }
    else if(argc == 2){
      if(argv[1][0]!='-'){
        printf("Second parameter is unknown");
        exit(EXIT_FAILURE);
      }

      char *p = (char*)(argv[1]+1);
      int flag = 0, op_a = 0, op_l=0;;

      while(*p){
        flag =1;

        if(*p != 'a' && *p != 'l'){
          printf("Unknown option\n");
          exit(EXIT_FAILURE);
        }

        if(*p == 'a'){
          op_a = 1;
        }
        
        if(*p == 'l'){
          op_l = 1;
        }

        p++;
      }

      if(flag==0){
        printf("Invalid second parameter");
        exit(EXIT_FAILURE);
      }

      _ls(".",op_a,op_l);
    }
    else{
      perror("Invalid parameter");
    }
}
