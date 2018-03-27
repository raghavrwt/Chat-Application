#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#define MAX	100

static unsigned int cli_count = 0;
static int uid = 10;

typedef struct {
	struct sockaddr_in addr;	
	int connfd;			
	int uid;			
	char name[32];			
} client_t;

client_t *clients[MAX];

void add(client_t *cl)
{
	int i;
	for(i=0;i<MAX;i++)
	{
		if(!clients[i])
		{
			clients[i] = cl;
			return;
		}
	}
}

void delete(int uid)
{
	int i;
	for(i=0;i<MAX;i++)
	{
		if(clients[i])
		{
			if(clients[i]->uid == uid)
			{
				clients[i] = NULL;
				return;
			}
		}
	}
}
void send_message(char *s)
{
	
	int i;
	for(i=0;i<MAX;i++)
	{
		if(clients[i])
		{
			write(clients[i]->connfd, s, strlen(s));
		}
	}
}


void strip_newline(char *s)
{
	while(*s != '\0'){
		if(*s == '\r' || *s == '\n')
		{
			*s = '\0';
		}
		s++;
	}
}

void print_client_addr(struct sockaddr_in addr)
{
	printf("%d.%d.%d.%d",
		addr.sin_addr.s_addr & 0xFF,
		(addr.sin_addr.s_addr & 0xFF00)>>8,
		(addr.sin_addr.s_addr & 0xFF0000)>>16,
		(addr.sin_addr.s_addr & 0xFF000000)>>24);
}

void *hanle_client(void *arg)
{
	char buff_out[1024];
	char buff_in[1024];
	int rlen;

	cli_count++;
	client_t *cli = (client_t *)arg;

	printf("<<ACCEPT ");
	print_client_addr(cli->addr);
	printf(" REFERENCED BY %d\n", cli->uid);

	sprintf(buff_out, "<<JOIN, HELLO %s\r\n", cli->name);
	send_message(buff_out);

	bzero(buff_in,sizeof(buff_in));
	while((rlen = read( cli->connfd,buff_in,sizeof(buff_in)-1))>0)
	{
		
			//rlen = read(cli->connfd, buff_in, 1024);
			//printf("[%s] %s",cli->name,buff_in);
			buff_out[0] = '\0';
			memset(&buff_out[0], 0, sizeof(buff_out));
			strip_newline(buff_in);
			if(!strlen(buff_in))
			{
				continue;
			}
			bzero(buff_out,sizeof(buff_out));
			printf("[%s] %s\n",cli->name,buff_in);
			sprintf(buff_out, "[%s] %s\r\n", cli->name, buff_in);
			send_message(buff_out);
			memset(&buff_in[0], 0, sizeof(buff_in));
			
		
		
	}
	close(cli->connfd);
	/* Send message*/
	sprintf(buff_out, "<<LEAVE, BYE %s\r\n", cli->name);
	send_message(buff_out);

	/* Delete client from queue and yeild thread */
	delete(cli->uid);
	printf("<<LEAVE ");
	print_client_addr(cli->addr);
	printf(" REFERENCED BY %d\n", cli->uid);
	free(cli);
	cli_count--;
	pthread_detach(pthread_self());
	
	return NULL;
}

int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0, portno;
	struct sockaddr_in server;
	struct sockaddr_in client;
	pthread_t tid;
	if (argc < 2) {
         printf("ERROR, no port provided\n");
         exit(1);
     }
     
     
    //Create socket
    listenfd= socket(AF_INET , SOCK_STREAM , 0);
    if (listenfd == -1)
    {
        printf("Could not create socket");
    }
    bzero((char *) &server, sizeof(server));
    portno = atoi(argv[1]);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portno);
     
	/* Bind */
	if(bind(listenfd, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		perror("Socket binding failed");
		return 1;
	}

	/* Listen */
	if(listen(listenfd, 10) < 0){
		perror("Socket listening failed");
		return 1;
	}

	printf("<[SERVER STARTED]>\n");
	socklen_t clilen = sizeof(client);

	/* Accept clients */
	//while(1){
	while( (connfd = accept(listenfd, (struct sockaddr *)&client, (socklen_t*)&clilen)))
	{

		/* Check if max clients is reached */
		if((cli_count+1) == MAX){
			printf("<<MAX CLIENTS REACHED\n");
			printf("<<REJECT ");
			print_client_addr(client);
			printf("\n");
			close(connfd);
			continue;
		}

		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->addr = client;
		cli->connfd = connfd;
		cli->uid = uid++;
		sprintf(cli->name, "%d", cli->uid);

		/* Add client to the queue and fork thread */
		add(cli);
		pthread_create(&tid, NULL, &hanle_client, (void*)cli);

		/* Reduce CPU usage */
		sleep(10);
	}
}
