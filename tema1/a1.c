#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

typedef struct
{
    char name[8];
    int type;
    int offset;
    int size;
} sectHeader;

void list(char *dirPath, int recursiv, char *name_ends_with, int permissions);
int octal(int binaryNo);

void parse(char *dirPath);
void findall(char *dirPath);

int main(int argc, char **argv)
{

    if (argc >= 2)
    {
        if (strcmp(argv[1], "variant") == 0)
        {
            printf("42307\n");
        }

        if (strcmp(argv[1], "list") == 0)
        {
            int recursiv = 0;
            char *name_ends_with = NULL, *permissions = NULL, *path = NULL;

            printf("SUCCESS\n");

            int k = 2;
            while (k < argc)
            {
                if (strncmp(argv[k], "path=", 5) == 0)
                {
                    path = argv[k] + 5;
                }
                else if (strcmp(argv[k], "recursive") == 0)
                {
                    recursiv = 1;
                }
                else if (strncmp(argv[k], "name_ends_with=", 15) == 0)
                {
                    name_ends_with = argv[k] + 15;
                }
                else if (strncmp(argv[k], "permissions=", 12) == 0)
                {
                    permissions = argv[k] + 12;
                }
                k++;
            }

            //transformare permisiuni in sir de biti de 1 si 0
            if (permissions != NULL)
            {
                for (int i = 0; i < strlen(permissions); i++)
                {
                    if (permissions[i] == '-')
                    {
                        permissions[i] = '0';
                    }
                    else
                    {
                        permissions[i] = '1';
                    }
                }

                //transformare din string in int
                int permissionsInt = 0;
                permissionsInt = atoi(permissions);

                list(path, recursiv, name_ends_with, octal(permissionsInt));
            }
            else if (permissions == NULL)
            {
                list(path, recursiv, name_ends_with, 0);
            }
        }

        if (strcmp(argv[1], "parse") == 0)
        {
            if (argc == 3)
            {
                char *path = argv[2] + 5;

                parse(path);
            }
            else
            {
                printf("ERROR\n Path not specified");
            }
        }

        if (strcmp(argv[1], "findall") == 0)
        {
            if (argc == 3)
            {
                char *path = argv[2] + 5;
                printf("SUCCESS\n");
                findall(path);
            }
            else
            {
                printf("ERROR\n Path not specified");
            }
        }
    }
    return 0;
}

//functie care imi transforma un nr binar in unul octal, returnandu-mi nr in decimal
int octal(int binaryNo)
{
    int octalNo = 0, j = 1;
    while (binaryNo != 0)
    {
        int aux = binaryNo % 10;
        octalNo = octalNo + aux * j;
        j = j * 2;
        binaryNo = binaryNo / 10;
    }
    return octalNo;
}

void list(char *dirPath, int recursiv, char *name_ends_with, int permissions)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;
    char fullPath[512];

    dir = opendir(dirPath);
    if (dir == NULL)
    {
        perror("invalid directory path");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", dirPath, entry->d_name);

            if (lstat(fullPath, &statbuf) == 0)
            {
                if (name_ends_with != NULL && strcmp(fullPath + strlen(fullPath) - strlen(name_ends_with), name_ends_with) == 0)
                {

                    printf("%s\n", fullPath);
                }

                else if (permissions != 0 && ((statbuf.st_mode & 0777) == permissions))
                {
                    printf("%s\n", fullPath);
                }
                else if (permissions == 0 && name_ends_with == NULL)
                {
                    printf("%s\n", fullPath);
                }

                if (S_ISDIR(statbuf.st_mode) && recursiv == 1)
                {
                    list(fullPath, recursiv, name_ends_with, permissions);
                }
            }
        }
    }
    closedir(dir);
}

void parse(char *dirPath)
{

    char magic;
    int header_size = 0;
    int version = 0;
    int no_of_sections = 0;

    int fd = open(dirPath, O_RDONLY);
    if (fd == -1)
    {
        perror("invalid directory path");
        return;
    }

    //magic
    lseek(fd, -1, SEEK_END);
    read(fd, &magic, 1);
    if (magic != 'l')
    {
        printf("ERROR\nwrong magic");
        exit(0);
    }

    //header_size
    lseek(fd, -3, SEEK_END);
    read(fd, &header_size, 2);

    //ne deplasam la inceputul header-ului
    lseek(fd, -header_size, SEEK_END);

    //version
    read(fd, &version, 4);

    if (version < 109 || version > 206)
    {
        printf("ERROR\n");
        printf("wrong version");
        exit(0);
    }


    //no_of_sections
    read(fd, &no_of_sections, 1);
    if (no_of_sections < 7 || no_of_sections > 17)
    {
        printf("ERROR");
        printf("\nwrong sect_nr");
        exit(1);
    }

    //sections
    sectHeader s;
    memset(&s, 0, sizeof(sectHeader));

    for (int i = 0; i < no_of_sections; i++)
    {
        read(fd, &s.name, 8);
        read(fd, &s.type, 1);
        if (s.type != 22 && s.type != 19 && s.type != 35)
        {
            printf("ERROR");
            printf("\nwrong sect_types");
            exit(0);
        }
        read(fd, &s.offset, 4);
        read(fd, &s.size, 4);
    }

    lseek(fd, -header_size + 5, SEEK_END);

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d", no_of_sections);

    for (int i = 0; i < no_of_sections; i++)
    {
        read(fd, &s.name, 8);
        //punem '\0' la finalul stringului
        char str2[9];
        memcpy(str2, s.name, 8 * sizeof(char));
        str2[9] = 0;//

        read(fd, &s.type, 1);
        read(fd, &s.offset, 4);
        read(fd, &s.size, 4);

        printf("\nsection%d: %s %d %d", i + 1, str2, s.type, s.size);
    }

    close(fd);
}

void findall(char *dirPath)
{

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;
    char fullPath[512];

    dir = opendir(dirPath);
    if (dir == NULL)
    {
        perror("ERROR\ninvalid directory path");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {

        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(fullPath, 512, "%s/%s", dirPath, entry->d_name);

            int nr = 0, nr1 = 0;

            char magic;
            int header_size = 0;
            int version = 0;
            int no_of_sections = 0;
            sectHeader s;

            int fd = open(fullPath, O_RDONLY);

            if (fd == -1)
            {
                perror("invalid file path");
                return;
            }
            //magic
            lseek(fd, -1, SEEK_END);
            read(fd, &magic, 1);
            if (magic == 'l')
            {

                //header_size
                lseek(fd, -3, SEEK_END);
                read(fd, &header_size, 2);
                lseek(fd, -header_size, SEEK_END);

                //version
                read(fd, &version, 4);

                if (version >= 109 && version <= 206)
                {

                    //no_of_sections
                    read(fd, &no_of_sections, 1);
                    if (no_of_sections >= 7 && no_of_sections <= 17)
                    {

                        memset(&s, 0, sizeof(sectHeader));

                        for (int i = 0; i < no_of_sections; i++)
                        {
                            read(fd, &s.name, 8);
                            read(fd, &s.type, 1);
                            if (s.type == 22 || s.type == 19 || s.type == 35)
                            {
                                nr++;
                            }
                            read(fd, &s.offset, 4);
                            read(fd, &s.size, 4);
                        }
                    }
                }
            }

            if (nr == no_of_sections) ///daca e valid fisierul, ne uitam la dimensiunea sectiunilor acestuia 
            {
                lseek(fd, -header_size + 5, SEEK_END);
                for (int i = 0; i < no_of_sections; i++)
                {
  
                    read(fd, &s.name, 8);
                    read(fd, &s.type, 1);
                    read(fd, &s.offset, 4);
                    read(fd, &s.size, 4);
                    if (s.size <= 1182)
                        nr1++;
                }
 			}

            if (lstat(fullPath, &statbuf) == 0)
            {
            		//daca fiserul detine toate cerintele cerute, afisam calea

                if (nr1 == no_of_sections && S_ISDIR(statbuf.st_mode) == 0 && no_of_sections!=0)
                {

                    printf("%s\n", fullPath);
                }

                if (S_ISDIR(statbuf.st_mode))
                {
                    findall(fullPath);
                }
            }
        

            close(fd);
        }
    }
    closedir(dir);
}
