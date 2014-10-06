// Marcel Marki & Kyle Nolan
// GPS Parsing Module Library File

#include "parse.h"

// Use extern keyword in main.c
char *string;
char *sentence;
char *gps_data[12];

//Smartly manipulate the NMEA string based on input value.
int merge_nmea(char in_char){

	if (in_char != '\r' && in_char != '\n')
	{
		//Not an end character
		string[strlen(string)] = in_char;
		
		//Sentence Check
		if(strlen(string) == 6){
			if(strcmp(string,"$GPRMC") == 0)
				printf("Accepted\n");
			else {
				printf("Rejected\n");
				free(string);
				string = (char *) malloc(70);
			}
		}
	}
	else{
		printf("string before death: %s\n",string);	
		printf("end of sentence \n");
		
		strcpy(sentence,string);
	
		free(string);
		string = (char *) malloc(70);
	}
	//printf("%s\n",string);
	
	return 0;
}

//Parse the NMEA String
int parse_nmea(char *in_sent){
	int i = 0;
	char* token;
	
	//Error case for string. Check sentence token
	if (!tag_check(in_sent)){
		printf("Error, no appropriate NMEA tag.\n");
		return -1;
	}
	else{
		//Begin parsing in full

		// get the first token 
		token = strtok_single(in_sent, ",");
		gps_data[0] = token;
		/* walk through other tokens */
		//The first call you use a char array which has the elements you want parsed.
		//The second time you call it you pass it NULL as the first parameter to tell function 
		//to resume from the last spot in the string. Once the first call is made your char 
		//array receives the parsed string. If you don't put NULL you would lose your place 
		//and effectivly the last part of your string.

		while(token) 
		{
			i++;
			token = strtok_single(NULL, ",");
			//printf( "%d, %s\n", i, token);
			gps_data[i] = token;
		} 
		for(i =0; i<13;i++) printf("%d: %s\n", i, gps_data[i]);
	}

	return 0;
}

//Function to check if $GPRMC tag is present
int tag_check(char *in_sent){
	char token[6] = "$12345";
	int x = 0;
	printf("in_sent: %s\n",in_sent);

	for (x = 0; x <6; x ++){
	        token[x] = in_sent[x];
	}
	if (strcmp(token,"$GPRMC") ==0) return 1;
	else return 0;
	return 1;
}

//Free and reallocate string memory to clear string.
//Is there an easier way?
//Error checking?
int reallocate(char *in_string, int byte_num){

	free(in_string);
	in_string = (char *) malloc(byte_num);
	return 1;
}

//Tokenize the String (one token per function call)
char *strtok_single (char * in_str, char const * delims)
{
  static char  * src = NULL;

  char  *  p,  * ret = 0;

  if (in_str != NULL)
    src = in_str;

  if (src == NULL)
    return NULL;

  if ((p = strpbrk (src, delims)) != NULL) {
    *p  = 0;
    ret = src;
    src = ++p;
  }

  return ret;
}

//Prepare the Sentence for Parsing
char * sent_prep(char* in_sent){
	//strip

	//concatenate
	in_sent = strcat(in_sent, ",");

	return in_sent;
}