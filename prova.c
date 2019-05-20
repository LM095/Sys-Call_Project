#include<stdio.h>
#include<string.h>
#include<stdlib.h>



struct request
{
	char id[255 + 1];
	char service[255 + 1];
};

int main(void)
{
	const char culo[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque eget venenatis dui. Sed lacus ipsum, vestibulum sed faucibus sit amet, aliquam et sem. Nulla in ex sit amet massa pretium lacinia ac vel augue. Duis malesuada eu lorem eu sodales. Morbi ac iaculis arcu. Ut metus magna, euismod sit amet lacus vel, sollicitudin congue lectus. Fusce in pharetra ipsum. Vestibulum blandit nisi eget justo malesuada semper. Maecenas sagittis magna orci, ut semper sem luctus eu. Aenean pulvinar, augue sit amet vehicula eleifend, massa nibh convallis mi, quis volutpat sapien erat eu mi. Maecenas malesuada, libero id vulputate aliquet, sem eros consectetur nibh, vel ultricies quam ligula et neque. Ut iaculis, augue quis blandit suscipit, lectus purus elementum dui, eu porta sem est eu lorem. Maecenas commodo placerat odio non aliquet. Aliquam ac vulputate dolor.";
	struct request r1;
	char id[255 + 1];
	printf("Inserire l'id: \n");
	//scanf("%255[^\n]s",id);
	/*
	int i=0;
	while(id[i] != '\0')
	{
		printf("%c",id[i]);
		i++;
	}
	*/
	

	printf("\n");

	strncpy(r1.id,culo, 255);

	printf("%s\n\n",r1.id);
	return 0;
}