#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string>
#include <cstring>
#include <sqlite3.h>
using namespace std;
/* portul folosit */
#define PORT 2908
fd_set Clients;

/* codul de eroare returnat de anumite apeluri */
extern int errno;
sqlite3 *db;  // descriptorul bazei de date 
char *zErrMsg = 0; // mesajele de eroare de la sql
int rc,MAXFD; // codul returnat de interogari
char BUFFERVITEZA[100];
char BUFFERSTRADA[100];
const char* data = "Callback function";



typedef struct thData
{
	int idThread; //id-ul thread-ului care va deservi clientul
	int cl; //descriptorul intors de accept
	int viteza;//viteza de circulatie
	char LicensePlate[100];//placuta de inmatriculare ,identificare unica;
	char strada[100]; // strada pe care circula un client la un moment dat
	char plecare[100]; // punctul de plecare
	char destinatie[100]; //destinatia
}thData;

struct EVENIMENT    //retin evenimentele 
 {
 	char tip[100]; // tipul evenimentului
 	char strada[100]; // strada pe care a fost raportat evenimentul
 	char raportatDe[100]; //cine a raportat evenimentul
 };
 EVENIMENT LastEvent; 
 

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde (void *);



static int Parcurs(void *data, int argc, char **argv, char **azColName) // Pentru a returna parcursul in functie de punctul A si B
{  
   int i;
   char buffer[100],final[100];
   bzero(buffer,100);
   bzero(final,100);
   for(i=0; i<argc; i++)
   {
      sprintf(buffer,"%s,", argv[i] ? argv[i] : "");  // formatez parcursul in format strada1,strada2,..,strada n
      strcat(final,buffer);
   }
   int payload;
   payload=strlen(final);
   if (write (*(int *)data, &payload, sizeof (int) ) <= 0)  // scriu payload-ul catre client 
  		{
    		perror ("[server]Eroare la write() catre client.\n");
   		}
   if (write (*(int *)data, final, payload) <= 0)  // scriu raspunsul catre client 
  		{
    		perror ("[server]Eroare la write() catre client.\n");
   		}
   return 0;
}

static int CheckSelect(void *data, int argc, char **argv, char **azColName) // verificam daca o interogare returneaza linii
{
	if (argv==NULL) {return 0;} else {return 4;}  
}

static int Peco(void *data, int argc, char **argv, char **azColName)  // pentru a trimite infromatii cu privire la Statii peco 
{  
    //if (argv==NULL) {return 0;} else {return 4;}  
   int i;
   char buffer[100],final[100];
   bzero(buffer,100);
   bzero(final,100);
   for(i=0; i<argc; i++)
   {
      sprintf(buffer,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      strcat(final,buffer);
   }
   
   char final2[100];
   strcpy(final2,"#");
   strcat(final2,final);
   int payload;
   payload=strlen(final2);
   if (write (*(int *)data, &payload,sizeof(int)) <= 0)  // scriu payload-ul catre client 
  		{
    		perror ("[server]Eroare la write() catre client.\n");
   		}
   if (write (*(int*)data, final2,payload) <= 0) // scriu raspunsul la client
      {
        perror ("[server]Eroare la write() catre client.\n");
            
    }
   return 0;
}
static int UpdateAccident(void *data, int argc, char **argv, char **azColName)  // pentru setare/resetarea Accident
{
	   return 0;
}
static int Sport(void *data, int argc, char **argv, char **azColName) // Pentru a returna informatii despre sport
{  
   int i;
   char buffer[100],final[100],buffer1[100];
   bzero(buffer,100);
   bzero(buffer1,100);
   bzero(final,100);
   for(i=0; i<argc; i++)
   {
      sprintf(buffer,"%s\n", argv[i] ? argv[i] : "");
      strcat(final,buffer);
   }
   bzero(buffer1,100);
   strcpy(buffer1,"Stiri Sport:\n");
   strcat(buffer1,final);
   int payload;
   payload=strlen(buffer1);
   if (write (*(int *)data, &payload,sizeof(int)) <= 0)  // scriu payload-ul catre client 
  		{
    		perror ("[server]Eroare la write() catre client.\n");
   		}
   if (write (*(int*)data, buffer1, payload) <= 0) // scriu raspunsul la client
      {
        perror ("[server]Eroare la write() catre client.\n");
            
    }
   return 0;
}
static int Vreme(void *data, int argc, char **argv, char **azColName)  // pentru a trimite infromatii cu privire la vreme
{  
    
   int i;
   char buffer[100],final[100],buffer1[100];
   bzero(buffer,100);
   bzero(buffer1,100);
   bzero(final,100);
   for(i=0; i<argc; i++)
   {
      sprintf(buffer,"%s : %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      strcat(final,buffer);
   }
   strcpy(buffer1,"Stiri Meteo:\n");
   strcat(buffer1,final);
   //printf("%s\nLungimea %d\n",buffer1,strlen(buffer1));
   int payload;
   payload=strlen(buffer1);
   if (write (*(int *)data, &payload,sizeof(int)) <= 0)  // scriu payload-ul catre client 
  		{
    		perror ("[server]Eroare la write() catre client.\n");
   		}
   if (write (*(int*)data, buffer1, payload) <= 0) // scriu raspunsul la client
      {
        perror ("[server]Eroare la write() catre client.\n");
            
    }
   return 0;
}
static int Viteza(void *data, int argc, char **argv, char **azColName) // trimite clientului mesajul corespunzator in functie de viteza de circulatie 
{  
   int i;
   char buffer[100],final[500];
   bzero(buffer,100);
   bzero(final,500);
   for(i=0; i<argc; i++)
   {
      sprintf(buffer,"%s", argv[i] ? argv[i] : "");
      strcat(final,buffer);
   }
   //printf("%s\n",final);
   bzero(BUFFERVITEZA,100);
   if(atoi(final)<*(int*)data)
   	sprintf(BUFFERVITEZA,"$Circulati cu o viteza prea mare!Puteti circula cu %dKm/h\n",atoi(final));
   else sprintf(BUFFERVITEZA,"$Respectati restrictia de viteza.(Maxim %d Km/h)\n",atoi(final));
   	//printf("%s",buffer);
   return 0;
}

static int Strada(void *data, int argc, char **argv, char **azColName) // trimite clientului limitarea de viteza de pe noua strada 
{  
   int i;
   char buffer[100],final[500];
   bzero(buffer,100);
   bzero(final,500);
   for(i=0; i<argc; i++)
   {
      sprintf(buffer,"%s", argv[i] ? argv[i] : "");
      strcat(final,buffer);
   }
   //printf("%s\n",final);
   bzero(BUFFERSTRADA,100);
   
    sprintf(BUFFERSTRADA,"@Am intrat pe alta strada.Noua limita de viteza e %d\n",atoi(final));
  
    //printf("%s",buffer);
   return 0;
}
 
void UpdateViteza( thData * client)   // functie apelata cand un client trimite  viteza de deplasare
{
   int VitezaNoua,payload;
   if (read (client->cl, &payload, sizeof(int)) <= 0)
         {
   			 perror ("[server]Eroare la read(UpdateViteza) de la client.\n\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
         return;
    		   
  		 }
   if (read (client->cl, &VitezaNoua, payload) <= 0)
         {
   			 perror ("[server]Eroare la read(UpdateViteza) de la client.\n\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
         return;
    		   
  		 }
   client->viteza=VitezaNoua;
   char raspuns[100];
   bzero(raspuns,100);
   string query,stradaC;
   query="select LimitaViteza-(Restrictie*AreAccident) from Strazi where NumeStrada='";
   stradaC=client->strada;
   query=query+stradaC+"'";
   rc = sqlite3_exec(db, query.c_str(), Viteza, (void*)&client->viteza, &zErrMsg);
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
   else
   {
      fprintf(stdout, "Am gasit viteza in BD\n");
   }
   payload=strlen(BUFFERVITEZA);
   if (write (client->cl, &payload, sizeof(int)) <= 0)
      {
        perror ("[server]Eroare la write() catre client.\n");
            
      }
   if (write(client->cl,BUFFERVITEZA,payload) <= 0)
      {
        perror ("[server]Eroare la write() catre client.\n");
            
      }
   
  	
  	
}
void UpdateStrada(thData * client)
{
   char * stradaNoua;

   int payload;

   if(read(client->cl,&payload,sizeof(int))<=0)
   {
   		perror ("[server]Eroare la read(UpdateStrada) de la client.\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
   }
   stradaNoua=(char*)calloc(payload,sizeof(char)); 
   if(read(client->cl,stradaNoua,payload)<=0)
   {
   		perror ("[server]Eroare la read(UpdateStrada) de la client.\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
   }
   strcpy(client->strada,stradaNoua);
   string query,stradaC;
   query="select LimitaViteza-(Restrictie*AreAccident) from Strazi where NumeStrada='";
   stradaC=client->strada;
   query=query+stradaC+"'";
   rc = sqlite3_exec(db, query.c_str(), Strada,NULL, &zErrMsg);
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
   else
   {
      fprintf(stdout, "Am gasit viteza in BD\n");
   }
   payload=strlen(BUFFERSTRADA);
   if (write (client->cl, &payload, sizeof(int)) <= 0)
      {
        perror ("[server]Eroare la write() catre client.\n");
            
      }
   if (write(client->cl,BUFFERSTRADA,payload) <= 0)
      {
        perror ("[server]Eroare la write() catre client.\n");
            
      }
    
 }
 

void RaporteazaEveniment(thData *  client)
{
   char *tip,*stradaev;
   int payload;
   
   if(read(client->cl , &payload, sizeof(int) )<=0)
   {
   		perror ("[server]Eroare la read payload (tip) de la client.\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
   }
   tip=(char*)calloc(payload,sizeof(char)); 
   if(read(client->cl,tip,payload)<=0)   // preluam tipul evenimentului
   {
   		perror ("[server]Eroare la read(tip) de la client.\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
   }
    if(read(client->cl,&payload,sizeof(int))<=0)
   {
   		perror ("[server]Eroare la read payload (Stradaev) de la client.\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
   }
   stradaev=(char*)calloc(payload,sizeof(char)); 
    if(read(client->cl,stradaev,payload)<=0)  // preluam strada pe care s-a raportat evenimentul
   {
   		perror ("[server]Eroare la read(Stradaev) de la client.\n");
   			 close (client->cl); /* inchidem conexiunea cu clientul */
   }
  
   strcpy(LastEvent.tip,tip);  // retinem ultimul eveniment
   strcpy(LastEvent.strada,stradaev);
   strcpy(LastEvent.raportatDe,client->LicensePlate);
   char buffer[100];
   sprintf(buffer,"%s a raportat %s pe strada %s\n",LastEvent.raportatDe,LastEvent.tip,LastEvent.strada);
   payload=strlen(buffer);
   for( int fd = 0; fd <=MAXFD; fd++ )
    if ( FD_ISSET(fd, &Clients) )
         {
         	printf("Socketul %d este deschis\n",fd);
         if (write (fd,&payload,sizeof(int)) <= 0)
            {
        		perror ("[server]Eroare la write() catre client.\n");
            
             }
         if (write (fd, buffer, payload) <= 0)
            {
        		perror ("[server]Eroare la write() catre client.\n");
            
             }
         }
     string query,str;
     str=stradaev;
     if(strcmp(tip,"Accident")==0) // s-a raportat un accident
     {
 		   query="Update Strazi set AreAccident=1 where NumeStrada='"+str+"'";
 		   rc = sqlite3_exec(db, query.c_str(), UpdateAccident,NULL, &zErrMsg);
       if( rc != SQLITE_OK )
        {
         fprintf(stderr, "SQL error: %s\n", zErrMsg);
         sqlite3_free(zErrMsg);
        }
       else
        {
         fprintf(stdout, "S-a inregistrat accidentul pe strada %s\n",str.c_str());
        }
     }
     if(strcmp(tip,"Liber")==0) // toate blocajele au fost eliberate 
     {
     	 query="Update Strazi set AreAccident=0 where NumeStrada='"+str+"'";
     	 rc = sqlite3_exec(db, query.c_str(), UpdateAccident,NULL, &zErrMsg);
       if( rc != SQLITE_OK )
        {
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
        }  
       else
        {
         fprintf(stdout, "S-a eliberat accidentul pe strada %s\n",str.c_str());
        }
     } 

}

void InfoVreme(thData *  client)
{  
  srand(time(NULL));
  int numar ;
  numar=abs(rand()%5+1);
  string query="select \"Temperatura Minima\",\"Temperatura Maxima\",\"Sanse Precipitatii\" from StiriMeteo where ID=";
  char buffer[100];
  bzero(buffer,100);
  sprintf(buffer,"%d",numar);
  string id=buffer;
  query=query+id+";";
  rc = sqlite3_exec(db, query.c_str(), Vreme, (void*)&client->cl, &zErrMsg);
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
   else
   {
      fprintf(stdout, "Am preluat info vreme din DB\n");
   }




}
void InfoSport(thData *  client)
{
   srand(time(NULL));
  int numar ;
  numar=abs(rand()%3+1);
  string query="select Stire1,Stire2,Stire3 from StiriSport where ID=";
  char buffer[100];
  bzero(buffer,100);
  sprintf(buffer,"%d",numar);
  string id=buffer;
  query=query+id+";";
  rc = sqlite3_exec(db, query.c_str(), Sport, (void*)&client->cl, &zErrMsg);
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
   else
   {
      fprintf(stdout, "Stirile despre sport au fost preluate din BD\n");
   }

}
void InfoPeco(thData *  client)
{    
   string sql,stradacurenta;
   stradacurenta=client->strada;
   sql = "select Statii.Statie,StatiiStrazi.Distanta,Statii.Benzina,Statii.Motorina from Statii join \
   StatiiStrazi on Statii.Statie=StatiiStrazi.Statie join Strazi on Strazi.NumeStrada=StatiiStrazi.NumeStrada where StatiiStrazi.NumeStrada='"+stradacurenta+"'";
   rc=sqlite3_exec(db, sql.c_str(), CheckSelect, NULL, &zErrMsg);
   if(rc==4)   // daca interogarea returneaza linii se va apela Peco care va trimite catre client datele necesare
    {
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql.c_str(), Peco, (void*)&client->cl, &zErrMsg);
   if( rc != SQLITE_OK )
   {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
   else
   {
      fprintf(stdout, "Statiile Peco au fost preluate din BD\n");
   }
    }
    
    else  // interogarea nu returneaza linii,deci nu exista benzinarii pe acea strada
    {
     char buffer[100];
    bzero(buffer,100);
    sprintf(buffer,"#Nu exista benzinarii pe strada pe care circulati!\n");
    int  payload=strlen(buffer);
    if (write (client->cl, &payload, sizeof(int)) <= 0)
      {
        perror ("[server]Eroare la write() catre client.\n");
            
      }
    if (write (client->cl, buffer, payload) <= 0)
      {
        perror ("[server]Eroare la write() catre client.\n");
            
      }
    }

}


int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
   rc = sqlite3_open("Monitorizare.db", &db);
   if( rc )
   {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   
   }
   else
   {
      fprintf(stderr, "Suntem conectati la BD\n");

   }
 
  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      socklen_t length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

	
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	if(client>MAXFD)
		MAXFD=client;
        /* s-a realizat conexiunea */
    
	int idThread; //id-ul threadului
	int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;
	FD_SET(td->cl,&Clients);  // cand se conecteaza un client ii adaugam descriptorul la setul cu descriptori activi

	pthread_create(&th[i], NULL, &treat, td);	      // am creeat threadul si i-am dat functia pe care trebuie sa o trateze
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		FD_CLR(((struct thData*)arg)->cl,&Clients); // cand se deconecteaza un client ,ii scoatem descriptorul din fd_Set-ul cu descriptori activi 
		close (((struct thData*)arg)->cl);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
    int nr, i=0,payload;
    char  *street,*licencePlate,*speed,*dest,*plecare;		//mesajul primit de la client 
	struct thData client; 
	client= *((struct thData*)arg);
	

   printf ("[server]Asteptam punctul de plecare...\n");
   fflush (stdout);

    
   if (read (client.cl, &payload,sizeof(int)) <= 0)
         {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		   }
      /* citirea punctului de plecare*/
   plecare=(char*)calloc(payload,sizeof(char));  //alocam memorie in functie de payload

   if (read (client.cl, plecare, payload) <= 0)
         {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		   }
   printf ("[server]Punctul de plecare a fost receptionat...%s\n",plecare);
  		 
  
   printf ("[server]Asteptam destinatia...\n");
   fflush (stdout);
      

   	if (read (client.cl, &payload,sizeof(int)) <= 0)
         {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		   }
      /* citirea Destinatiei*/
   dest=(char*)calloc(payload,sizeof(char));  //alocam memorie in functie de payload
   if (read (client.cl, dest, payload) <= 0)
       {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		 }
   printf ("[server]destinatia a fost receptionata...%s\n", dest);
   strcpy(client.destinatie,dest);
   

  
   printf ("[server]Asteptam placuta de inmatriculare...\n");
   fflush (stdout);
      
    if (read (client.cl, &payload,sizeof(int)) <= 0)
         {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		   }

      /* citirea mesajului placuta */
   licencePlate=(char*)calloc(payload,sizeof(char));  //alocam memorie in functie de payload
   if (read (client.cl, licencePlate, payload) <= 0)
       {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    	
  		 }
  
   printf ("[server]Placuta de inmatriculare a fost receptionata...%s\n", licencePlate);
      
   printf ("[server]Asteptam viteza...\n");
   fflush (stdout);
      

   	if (read (client.cl, &payload,sizeof(int)) <= 0)
         {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		   }
      /* citirea mesajului viteza */
   speed=(char*)calloc(payload,sizeof(char));  //alocam memorie in functie de payload
   if (read (client.cl, speed, payload) <= 0)
       {
   			 perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		 }
  	

  
  printf ("[server]Viteza a fost receptionata...%s\n", speed);

  
  

  strcpy(client.LicensePlate,licencePlate); // umplem structura thData cu informatiile clientului
  printf("placuta\n");
  strcpy(client.plecare,plecare);
  printf("plecare\n");
  strcpy(client.destinatie,dest);
  printf("destinatie\n");
  int viteza;
  viteza=atoi(speed);
  client.viteza=viteza;
  printf("viteza\n");



      printf ( "S-a conectat clientul cu socketul %d avand numarul de inmatriculare %s care pleaca de la  %s inspre %s cu viteza %d\n",client.cl,client.LicensePlate,client.plecare,client.destinatie,client.viteza);
     
      string query,start,stop;
      query="select ST1,ST2,ST3,ST4,ST5,ST6 from Rute where start='";  // query-ul pentru stabilirea traseului 
      start=client.plecare;
      stop=client.destinatie;
      query=query+start;
      query=query+"' and stop='"+stop+"'";
       
     
     rc = sqlite3_exec(db, query.c_str(), Parcurs, (void*)&client.cl, &zErrMsg);  // apelam query-ul care va trimite traseul catre client
     if( rc != SQLITE_OK )
     {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
     }
     else
     {
      fprintf(stdout, "Traseul a fost gasit in BD\n");
     }
    
     // citim strada pe care circula initial clientul
 
    printf("[server]Asteptam strada...\n");
    if(read(client.cl,&payload,sizeof(int))<=0)
    {
       perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */

    }
    street=(char*)calloc(payload,sizeof(char));

    if(read(client.cl,street,payload)<=0)
     {
         perror ("[server]Eroare la read() de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
     }
     strcpy(client.strada,street);
    printf("[server]Am primit strada :%s\n",client.strada);

  
    string stradaC;
  	query="select LimitaViteza-(Restrictie*AreAccident) from Strazi where NumeStrada='";
  	stradaC=client.strada;
  	query=query+stradaC+"'";
  	rc = sqlite3_exec(db, query.c_str(), Viteza, (void*)&client.viteza, &zErrMsg);
    if( rc != SQLITE_OK )
    {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }
   else
   {
      fprintf(stdout, "Viteza de pe strada initiala gasita in DB\n");
   } 
   payload=strlen(BUFFERVITEZA);
   printf("dimensiunea BUFFERVITEZA :%d\n",payload);
   if( (write(client.cl,&payload,sizeof(int))<=-1))
   {
       
       perror ("[server]Eroare la write() payload viteza catre client.\n");
        
   }
   if( (write(client.cl,BUFFERVITEZA,payload)<=-1))
   {
       
       perror ("[server]Eroare la write() viteza catre client.\n");
        
   }
  
          // vom deservi clientul pana cand acesta va parasi aplicatia 
       
        int opt;
        while(1)
        {    
        	 if(read(client.cl,&payload,sizeof(int)) <=0)
        	 { 
        	 	 perror ("[server]Eroare la read payload (opt) de la client.\n");
   			     close (client.cl); /* inchidem conexiunea cu clientul */
        	 }
        	 if (read (client.cl, &opt, payload) <= 0) // citim optiunea aleasa de client 
               {
   			 perror ("[server]Eroare la read(opt) de la client.\n");
   			 close (client.cl); /* inchidem conexiunea cu clientul */
    		
  		       }
  		
  		 if(opt==0)  //daca s-a ales aceasta optiune , am terminat, se va ajunge la close din treat
  		 	{  
          char * buffer;
          buffer=(char*)calloc(strlen("*EXIT"),sizeof(char));
          strcpy(buffer,"*EXIT");
          payload=strlen(buffer);
          //printf("scriu payload\n");
          if (write (client.cl, &payload,sizeof(int)) <= 0)
            {
              perror ("[server]Eroare la write() EXIT catre client.\n");
            
            }
          // printf("scriu mesaj\n");
          if (write (client.cl, buffer,payload) <= 0)
            {
              perror ("[server]Eroare la write() EXIT catre client.\n");
            
            }
  		 	 printf ( "S-a deconectat clientul cu socketul %d avand numarul de inmatriculare %s care pleaca de la  %s inspre %s cu viteza %d\n",client.cl,client.LicensePlate,client.plecare,client.destinatie,client.viteza);
  		 	 break;
  		 	}
            
            switch(opt)   // in functie de optiunea aleasa executam functia corespunzatoare  ; functiile primesc ca parametru intreaga structura ca sa aiba acces la toate datele membru;
            {
            	case 1:
            	{   
            		printf("Am primit comanda %d\n",opt);
            		UpdateViteza(&client); 
            		break;
            	}
            	case 2:
            	{   

            		printf("Am primit comanda %d\n",opt);
            		UpdateStrada(&client);
            		break;
            	}
            	case 3:
            	{
            		printf("Am primit comanda %d\n",opt);
            		RaporteazaEveniment(&client);
            		break;
            	}
            	case 4:
            	{
            		printf("Am primit comanda %d\n",opt);
            		InfoVreme(&client);
            		break;
            	}
            	case 5:
            	{
            		printf("Am primit comanda %d\n",opt);
            		InfoSport(&client);
            		break;
            	}
            	case 6:
            	{
            		printf("Am primit comanda %d\n",opt);
            		InfoPeco(&client);
            		break;
            	}

            }
  		 }
  		 

	}
