#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

typedef struct result
{
	char *filename;
	int count;
	char str_location[50];
} Result;

int get_file_size(FILE *fp);
void cut_filename(char *H);
int compare_with_size(const void *a, const void  *b);
char *cut_enter(char* str);
unsigned int file_number(char *file_directory);
int exact_search(char *string_pointer, char *find_str);

int main()
{
	char *file_directory = (char *)calloc(256, sizeof(char)); // 폴더 위치
	char *find_str = (char *)calloc(256, sizeof(char)); // 찾고 싶은 단어
	char *buffer;
	char *buffer2;
	char *str_ptr; // 어떤 파일에서 찾고 싶은 단어의 위치(포인터)
	int i = 0; // 총 파일 개수
	int j = 0;
	int k = 0; // 찾고 싶은 단어가 존재하는 파일 개수
	int file_size; // 어떤 파일의 전체 텍스트 길이
	char YN1 = 'Y';
	char YN2 = 'Y';
	Result *r;
	struct _finddata_t c_file;
	long hFile;

	printf("폴더 위치를 입력하세요 : ");
	scanf("%[^\n]s", file_directory);
	getchar();

	printf("찾고 싶은 단어를 입력하세요 : ");
	scanf("%[^\n]s", find_str);
	getchar();

	printf("대소문자를 구분하나요?(Y/N) : ");
	YN1 = getchar();
	getchar();

	printf("입력한 단어만 정확히 검색하나요?(Y/N) : ");
	YN2 = getchar();
	getchar();

	find_str = (char *)realloc(find_str, strlen(find_str) + 1);
	strcat(file_directory, "\\*.txt");

	if ((hFile = _findfirst(file_directory, &c_file)) == -1L)
	{
		switch (errno)
		{
		case ENOENT:
			printf(":: No text file in such directory ::\n"); break;
		case EINVAL:
			fprintf(stderr, "Invalid path name.\n"); exit(1); break;
		case ENOMEM:
			fprintf(stderr, "Not enough memory or file name too long.\n"); exit(1); break;
		default:
			fprintf(stderr, "Unknown error.\n"); exit(1); break;
		}
	}
	else
	{
		r = (Result *)calloc(file_number(file_directory), sizeof(Result));
		cut_filename(file_directory);
		do
		{
			strcat(file_directory, c_file.name);
			FILE *fp = fopen(file_directory, "rt");
			if (fp == NULL) return 1;

			file_size = get_file_size(fp);
			buffer = (char *)calloc(1, (size_t)file_size + 1);
			buffer2 = (char *)calloc(1, (size_t)file_size + 1);

			while (1) //파일 전체 내용을 buffer에 저장
			{
				buffer2 = fgets(buffer2, file_size, fp);
				if (feof(fp) != 0) break;
				else if (buffer2 == NULL) return 3;
				strcat(buffer, buffer2);
			} 

			if (YN1 == 'N' || YN1 =='n')
			{
				_strlwr(buffer);
				_strlwr(find_str);
			}

			// 조건에 맞는 단어를 찾으면 메모리를 할당
			str_ptr = strstr(buffer, find_str);
			while (str_ptr != NULL)
			{
				if (YN2 == 'n' || YN2 == 'N' || exact_search(str_ptr, find_str) == 1)
				{
					r[k].filename = (char *)calloc(1, strlen(c_file.name) + 1);
					strncpy(r[k].filename, c_file.name, strlen(c_file.name));
					r[k].filename[strlen(c_file.name) + 1] = NULL;

					strncpy(r[k].str_location, str_ptr, sizeof(r[k].str_location) - 1);
					r[k].str_location[sizeof(r[k].str_location) - 1] = NULL;

					r[k].count = 0;

					k++;
					break;
				}
				str_ptr = strstr(str_ptr + strlen(find_str), find_str);
			}
			
			// 이후 조건에 맞는 단어를 찾으면 count만 증가
			while (str_ptr != NULL)
			{
				if (YN2 == 'n' || YN2 == 'N' || exact_search(str_ptr, find_str) == 1) 
					r[k - 1].count++;
				str_ptr = strstr(str_ptr + strlen(find_str), find_str);
			}

			fclose(fp);
			free(buffer);
			free(buffer2);

			buffer = NULL;
			buffer2 = NULL;

			cut_filename(file_directory);
			i++;

		} while (_findnext(hFile, &c_file) != -1);
		_findclose(hFile);

		if (k == 0)
		{
			printf("\n:: No result\n\n");
		}
		else
		{
			r = (Result *)realloc(r, k * sizeof(Result));
			if (r == NULL) return 2;

			qsort(r, k, sizeof(Result), compare_with_size);
			printf("\n찾고 싶은 단어가 존재하는 파일 개수 : %d\n", k);
			printf("\nFile name                                          |  Count | First word preview\n");
			for (j = 0; j < k; j++)
			{
				printf("%-50s |%5d번 | ...%s...\n", r[j].filename, r[j].count, cut_enter(r[j].str_location));
			}
		}
		
		free(r);
		free(find_str);
		free(file_directory);
	}
	system("pause");
	return 0;
}

int get_file_size(FILE *fp)
{
	int size;
	long currPos = ftell(fp);

	fseek(fp, 0, SEEK_END);
	size = (int)ftell(fp);

	fseek(fp, currPos, SEEK_SET);

	return size;
}

void cut_filename(char *H)	//file_directory에서 맨 뒤의 파일명을 제거
{
	int i = 0;
	while (H[i])
	{
		i++;
	}
	while (H[i] != '\\')
	{
		i--;
	}
	H[i + 1] = '\0';
}

int compare_with_size(const void *a, const void  *b)	//qsort 내림차순 정렬
{
	Result *ptr_a = (Result *)a;
	Result *ptr_b = (Result *)b;

	if (ptr_a->count < ptr_b->count) return 1;
	else if (ptr_a->count == ptr_b->count) return 0;
	else return -1;
}

char *cut_enter(char* str) //str에 포함된 '\n'을 ' '으로 바꿈
{
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n') str[i] = ' ';
	}
	return str;
}

unsigned int file_number(char *file_directory)
{
	_finddata_t fd;
	long handle;
	int result = 1;
	unsigned int file_number = 0;

	handle = _findfirst(file_directory, &fd);
	if (handle == -1) return 0;

	while (result != -1)
	{
		file_number++;
		result = _findnext(handle, &fd);
	}
	_findclose(handle);

	return file_number;
}

int exact_search(char *str_ptr, char *find_str)
{
	if ((*(str_ptr - 1) < 65 || 90 < *(str_ptr - 1) && *(str_ptr - 1) < 97 || 122 < *(str_ptr - 1)) && (*(str_ptr + strlen(find_str)) < 65 || 90 < *(str_ptr + strlen(find_str)) && *(str_ptr + strlen(find_str)) < 97 || 122 < *(str_ptr + strlen(find_str)))) return 1;
	else return -1;
}
