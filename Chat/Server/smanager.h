#ifndef SAMANGER_H
#define SAMANGER_H

#include "user.h"
#include <unistd.h>
#include <vector>
#include "conversation.h"
#include <fstream>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <typeinfo>
#include <iostream>

#define PORT 707

/////////////////////////////////////////
/// Header FILE for server_manager clas
/////////////////////////////////////////

/// Server manager class
/// Contains available users
/// Contains active conversations
class SeManager
{
	/// List of available users
	std::vector<User*> users;
	/// List of conversations
	std::vector<Conversation*> conversations;
public:

  /// Base constructor, Creates user lsit from files
	/// path has to be specified
	/// conversations will remain empty here
	SeManager()
	{
	/// For random numbers
		srand (time(NULL));
		/// Directory names and pointers
		std::fstream userfile;
		userfile.open("users.txt");
		std::string id;
		/// Make users from user file
		while ( userfile >> id )
		{
			/// Get path from name
			std::string dirname = "users/" + id;
			/// Make user from file
			User *us = new User(dirname);
			/// Add to list
			users.push_back(us);
		}
		userfile.close();
		/// Build conversations from their respected files
		std::fstream confile;
		confile.open("conversations.txt");
		/// Open files, add objects to list
		while( confile >> id )
		{
			/// Get dirname
			std::string dirname = "conversations/" + id;
			/// Build object from file
			Conversation *co = new Conversation(dirname);
			/// Add to list
			conversations.push_back(co);
    }
		confile.close();
  }

	/// Destructor
	/// Clears up when done
	~SeManager()
  {
		std::vector<User*>::iterator us;
		std::vector<Conversation*>::iterator co;

		/// Delete dinamic objects
		for( us = users.begin(); us != users.end(); ++us ) delete *us;
		for( co = conversations.begin(); co != conversations.end(); ++co ) delete *co;
  }

	/// Create user with given credentials
	/// Put file in folder and add to list
	/// @param name - username
	/// @param psw - password
	void createuser( std::string name, std::string psw )
	{
		/// Create ID and add to list
		int ID;
    std::fstream test;
		do
		{

			if(test.is_open()) test.close();

			ID = rand()%100+100;
      std::string testpath = "/home/md21/Documents/Projektek/$Chat/SRC/Server/users/" + std::to_string(ID);
			test.open(testpath);
			if(test.is_open()) std::cout << ID;

		} while(test.is_open());

		std::fstream userfile("users.txt", std::ios::app);
		userfile << std::endl << ID;
    userfile.close();
	/// Get path and create file with that
	std::string path = "/home/md21/Documents/Projektek/$Chat/SRC/Server/users/" + std::to_string(ID);

	std::fstream userf(path, std::fstream::out);
	userf << name << std::endl << psw << std::endl << ID;
	userf.close();

	}

	/// Add user if user after file created or when logged in
	/// Param username and password of user
	/// @param n - username
	/// @param p - password
	int adduser( std::string n, std::string p )
	{
		/// Read from userlist until nothing left
		std::fstream userlist("users.txt");
		std::string id;
		/// Check if user exists
		while( userlist >> id )
		{
			std::fstream userfile("users/" + id);
			/// If open check if credentials are correct
			if(userfile.is_open())
			{
				userfile.close();
				User *uj = new User("users/" + id);
				/// If correct then add and exit
				if( uj->getPass()==p && uj->getName()==n )
				{
					users.push_back(uj);
					userlist.close();
					return 0;
				}
				delete uj;
			}
		}
		/// If no user found exit
		userlist.close();
		return 1;

	}

	/// Create conversation
	/// ID is generated randomly, let user put others in it
	void createconversation( std::string participants[], int n )
	{
		std::fstream clist("conversations.txt");
		std::string convid, newid;
		bool there;
		/// Check if conversation exists
		/// Add new  id to list and create file
		do
		{
			there = false;
			newid = std::to_string( rand()%89 + 10 );
			while(clist >> convid) {
				if(newid==convid) there = true;
		}
	}
	while( there );

		/// Close&open to add new id
		clist.close();
		clist.open("conversations.txt", std::ios::app);
		/// Add to conversation list and create file
		clist << std::endl << newid;

		std::fstream conversationfile("conversations/" + newid, std::ios::app);
		conversationfile << newid << std::endl;
		for( int i = 0 ; i < n; i++ ) conversationfile << participants[i] << std::endl;
		conversationfile << "[MESSAGES]" << std::endl;
		conversationfile.close();

	}

	/// Add conversation
	/// Given ID's source file will be opened
	/// @param ID - Conversation being opened
	void addconversation( int id )
	{
		/// Open conversation file
		Conversation* newconv = new Conversation( "conversations/" + std::to_string(id) );
    conversations.push_back(newconv);
  }

	/// Add message to conversation
	/// ID is given search list and add message
	/// @param id - ID of covnersation
	void addmessage( char* buf )
	{
		/// Calculate id
		int power = 1, i = 0, id = 0; /// Calculate the biggest base
		while(buf[++i]!=':') power*=10; /// Get id from message
		for(int j=0; j<i; j++, power=power/10) id+=(buf[j]-'0')*power;
		/// Create message
		std::string message = "";
		for(size_t k = i+2; k < 1024 && buf[k]!=0; k++) message+=buf[k];
		/// Add to conversation
		std::vector<Conversation*>::iterator it;
		for( it = conversations.begin(); it != conversations.end(); ++it )
								if((*it)->getId()==id) (*it)->addmessage(message) ;
	}

	/// Starts up server
	/// Reads command from port until being done
	void start()
	{

		fd_set master;
		fd_set read_fds; // Temporary list for select()-is
		int fdmax;
		int listener = socket( AF_INET, SOCK_STREAM, 0 );
		int opt = 1; // SETSOCKOPT desc
		int new_socket; // Socket for client
		char buf[1024]; /// Buffer for socket operations
		int inbytes;
		struct sockaddr_in address;
		struct sockaddr_in cl_addr;
		socklen_t len = sizeof(address);

		FD_ZERO(&master);
		FD_ZERO(&read_fds);

		if( listener >= 0 )
		{

			/// Set address values and clear sin_zero
			address.sin_family = AF_INET;
			address.sin_port = htons(PORT);
			address.sin_addr.s_addr = INADDR_ANY;
			//memset(&(address.sin_zero), '\0', 8 );

			if( setsockopt(listener, SOL_SOCKET,
								SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(int)) < 0 )
			{
				perror("Setsockopt failed");
				exit(EXIT_FAILURE);
			}

			if( bind( listener, (struct sockaddr*)&address, len ) < 0 )
			{
				perror("Couldn't bind to port");
				exit(EXIT_FAILURE);
			}

			if( listen( listener, 10 ) < 0 )
			{
				perror("Listening on port failed");
				exit(EXIT_FAILURE);
			}

			// Add listener to master set
			FD_SET(listener, &master);
			fdmax = listener;

			// Main loop
			for(;;)
			{
				read_fds = master;
			  if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
				{
			  	perror("select");
			    exit(1);
				}

				/// Check if connections has something to read
				for(int i = 0; i <= fdmax; i++)
				{
			  	if (FD_ISSET(i, &read_fds))
					{
			    	if (i == listener)
						{
			      	// Add new connection
			        len = sizeof(cl_addr);
			        if ((new_socket = accept(listener,
												(struct sockaddr *)&cl_addr,&len)) == -1)
							{
			        	perror("accept");
							}
							else
							{
			        	FD_SET(new_socket, &master); /// Add to master
			          if (new_socket > fdmax)
								{
			          	fdmax = new_socket; /// Dont't forget the maximum
								}
			          printf("selectserver: new connection from %s on " "socket %d\n",
													inet_ntoa(cl_addr.sin_addr), new_socket);
							}
						}
						else
						{
			      	/// Clien's data is missing
			        if ((inbytes = recv(i, buf, 1024, 0)) <= 0)
							{
			        	/// Treat error
			          if (inbytes == 0)
								{
			          	printf("selectserver: socket %d hung up\n", i); }
								else
								{
			          	perror("recv");
								}
			          close(i);

			          FD_CLR(i, &master); /// Remove from master set
							}
							else
							{
								addmessage(buf);
							}
						}
					}
			  }
		  }
		}
		else
		{
			perror("Couldn't open socket");
			exit(EXIT_FAILURE);
		}
		close( listener );
	}

	/// Dev purposes, useless otherwise
	void list()
	{
		std::cout << conversations.size() << std::endl;
		std::vector<Conversation*>::iterator it2;
		for( it2 = conversations.begin(); it2 != conversations.end(); ++it2 ) std::cout << (*it2)->getId() << std::endl;
	}

};

#endif
