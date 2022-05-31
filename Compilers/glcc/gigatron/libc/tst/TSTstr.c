#include <stdio.h>
#include <string.h>
#include <errno.h>

const char *str1 = "The quick brown fox jumps over the lazy dog";
char str2[512];
char str3[1024];

void fill_str2()
{
	int i;
	for (i=0; i<sizeof(str2);i++)
		str2[i] = '0' + (i % 10);
	str2[489] = 0;
	str2[123] = 'A';
	str2[258] = 'B';
	str2[259] = 'B';
	str2[358] = 'A';
}

const char *nullp(const char *s)
{
	return (s) ? s : "(null)";
}

int main()
{
	char *s;
	fill_str2();

	printf("str1=[%s] len=%d\n", str1, strlen(str1));
	printf("str2=[%s] len=%d\n", str2, strlen(str2));
	printf("strlen(str1+3)=%d\n", strlen(str1+3));
	printf("strlen(str2+255)=%d\n", strlen(str2+255));

	printf("strchr(str1,'o')=str1+%d\n", strchr(str1,'o')-str1);
	printf("strchr(str2,'A')=str2+%d\n", strchr(str2,'A')-str2);
	printf("strchr(str2,'B')=str2+%d\n", strchr(str2,'B')-str2);
	printf("strchr(str2,'C')=%#x\n", strchr(str2,'C'));

	printf("strrchr(str1,'o')=str1+%d\n", strrchr(str1,'o')-str1);
	printf("strrchr(str2,'A')=str2+%d\n", strrchr(str2,'A')-str2);
	printf("strrchr(str2,'B')=str2+%d\n", strrchr(str2,'B')-str2);
	printf("strrchr(str2,'C')=%#x\n", strrchr(str2,'C'));

	printf("strcpy(str3, str1)=\"%s\"\n", strcpy(str3, str1));

	printf("strcat(str3, str1)=\"%s\"\n", strcat(str3, str1));

	printf("strncat(str3, str2, 17)=\"%s\"\n", strncat(str3, str2, 17));

	printf("strncpy(str3, str2, 67)=\"%s\"\n", strncpy(str3, str2, 67));
	printf("strncpy(str3, str1, 67)=\"%s\"\n", strncpy(str3, str1, 67));
	printf("strncpy(str3, str1, 67)+50=\"%s\"\n", strncpy(str3, str1, 67)+50);

	printf("strcat(str3, strerror(ENOENT))=\"%s\"\n", strcat(str3, strerror(ENOENT)));

	printf("strstr(str1,\"brown\")=\"%s\"\n", strstr(str1,"brown"));
	printf("strstr(str1,\"brownfox\")=\"%s\"\n", nullp(strstr(str1,"brownfox")));
	s = strcpy(str3, str1);
	while(s = strtok(s," ")) {
		printf("strtok1: \"%s\" (+%d)\n", s, s-str3);
		s = 0;
	}

	s = strcpy(str3, str2);
	while(s = strtok(s,"0123456789")) {
		printf("strtok2: \"%s\" (+%d)\n", s, s-str3);
		s = 0;
	}
	
	return 0;
}
