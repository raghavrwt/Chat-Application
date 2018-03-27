#include<stdio.h> //printf
#include<string.h>    //strlen
#include<arpa/inet.h> //inet_addr
#include<netdb.h>
#include<stdlib.h>
#include<string.h>
#include<gtk/gtk.h>
int sockfd;
char server_reply[2000];
typedef struct
{
     GtkWidget *entry, *textArea;
     GtkTextBuffer *buffer;
} Widgets;

static void callback (GtkButton*, Widgets*);
//GtkWidget *textArea;
//GtkTextBuffer *buffer;
static void setUpGUI(int arc, char *arv[]);


int main(int argc , char *argv[])
{
	
	int  portno ;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    if (argc <3) 
    {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(1);
    }
	portno = atoi(argv[2]);

    //Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0)
   {
      perror("ERROR opening socket");
      exit(1);
   }
   server = gethostbyname(argv[1]);
   
   if (server == NULL) 
   {
      fprintf(stderr,"ERROR, no such host\n");
      exit(1);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   
   
    //Connect to remote server
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
   {
      perror("ERROR connecting");
      exit(1);
   }
   
    puts("Connected\n");
    
    
    
    setUpGUI(argc,argv);
    gtk_main();
    
    
    //keep communicating with server
    close(sockfd);
    
    return 0;
    
}
static void callback( GtkButton *button, Widgets *obj)
{
    const gchar *entry_text;
    //char message[2000];
    GtkTextMark *mark;
    GtkTextIter iter;
    obj->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(obj->textArea));
	entry_text = gtk_entry_get_text(GTK_ENTRY(obj->entry));
	//strcpy(message, entry_text);
	send(sockfd , entry_text, strlen(entry_text),0);

    bzero(server_reply,2000);
    recv(sockfd , server_reply , 2000,0);
    //printf("%s\n", server_reply);
    
	mark = gtk_text_buffer_get_insert(obj->buffer);
	gtk_text_buffer_get_iter_at_mark(obj->buffer, &iter, mark);
	gtk_text_buffer_insert(obj->buffer, &iter, server_reply, -1);
	gtk_entry_set_text(GTK_ENTRY(obj->entry), "");
    
}

static void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}

static void initialize_window(GtkWidget* window)
{
	gtk_window_set_title(GTK_WINDOW(window),"Messenger"); //Set window title
	gtk_window_set_default_size (GTK_WINDOW (window), 400, 200); //Set default size for the window
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	g_signal_connect (window, "destroy", G_CALLBACK (destroy), NULL); //End application when close button clicked
}
	

static void setUpGUI(int arc , char *arv[])
{
	GtkWidget *window,*box1,*box2,*scrolledwindow,*label1,*label2,*button;
	GtkEntryBuffer *entry_buffer= gtk_entry_buffer_new(NULL, 0);
	GtkTextBuffer *buffer1=gtk_text_buffer_new(NULL);
	gtk_init(&arc, &arv);
	
	bzero(server_reply,2000);
    recv(sockfd , server_reply , 2000,0);

	//Create the main window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	initialize_window(window);
	
	Widgets *obj = g_slice_new(Widgets);
	//Create new box
	box1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), box1);
	
	/* create a new label Message. */
	label1 = gtk_label_new("Message:" );
	gtk_misc_set_alignment(GTK_MISC (label1),0,0);
	gtk_box_pack_start(GTK_BOX(box1), label1, FALSE, FALSE, 0);
	gtk_widget_show(label1);
	
	//Create new TextView
	obj->textArea = gtk_text_view_new();
	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledwindow),obj->textArea);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(obj->textArea),FALSE);
	gtk_box_pack_start(GTK_BOX (box1), scrolledwindow, TRUE, TRUE, 0);
	
	gtk_text_buffer_set_text(buffer1, server_reply, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(obj->textArea), buffer1);
	gtk_widget_show(obj->textArea);
	gtk_widget_show(scrolledwindow);
	
	/* create a new label Enter message. */
	label2 = gtk_label_new("Enter message:" );
	gtk_misc_set_alignment(GTK_MISC (label2), 0, 0);
	gtk_box_pack_start(GTK_BOX (box1), label2, FALSE, FALSE, 0);
	gtk_widget_show(label2);
	
	box2 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX (box1), box2, FALSE, FALSE, 0);
	
	
	//create a text box
	obj->entry = gtk_entry_new_with_buffer(entry_buffer);
	gtk_box_pack_start(GTK_BOX (box2),obj->entry, FALSE, FALSE, 0);
	gtk_widget_show(obj->entry);
	
	
	
	//Create a new button
	button = gtk_button_new_with_label ("Send");
	g_signal_connect (G_OBJECT(button), "clicked",G_CALLBACK(callback),(gpointer) obj);	
	gtk_box_pack_start(GTK_BOX (box2), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	
	
	gtk_widget_show(box2);
	
	gtk_widget_show (box1);
    gtk_widget_show (window); 
    
    
    
      
}
