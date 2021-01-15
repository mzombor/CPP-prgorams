#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include "menu.h"
#include "user.h"
#include <unistd.h>

////////////////////////////
/// Header FILE client_manager.h
////////////////////////////

/// Manager class, de facto client
/// manages user input and server communiations
/// Doesn't really know anything yet
/// Will be expanded
class Manager {
	User user;
	State state = MAIN;
public:
	
	/// Manager constructor, sends signal to server
	/// if given credentials are correct, or throw error
	/// @param - username - given username
	/// @param - password - given password
	Manager( std::string username, std::string password ) {
		if( username == "rossz" ) throw std::out_of_range("rossz");
		user = User( username, password );
	}
	
	/// Manager destructor, sends signal user has stopped using it
	~Manager(){}
	
	/// Shows menu, depending on state
	/// Every scene is different
	void showmenu() const {
		if(state==MAIN) {
			system("clear");
			std::cout << "$$ Chat - Haligali!" << std::endl
			          << "- - - - - - - - - -" << std::endl << std::endl
			          << " 1 - Csatlakozas beszelgeteshez" << std::endl
					  << " 2 - Beszelgetes letrehozasa" << std::endl
					  << " 3 - Kilepes" << std::endl;
		}
		else if(state==JOIN) {
			system("clear");
			std::cout << "$$ Chat - Beszelgetesek" << std::endl
					  << "Valassz egy beszelgetest, kilepeshez '-1'" << std::endl
					  << "----------------------------" << std::endl << std::endl;
		}
		else if(state==CONVERSATION) {
			system("clear");
			std::cout << "$$ Chat - Beszeletes" << std::endl
					  << "Ird be az ueneted, kilepeshez q" << std::endl
					  << "-----------------------------" << std::endl << std::endl;
		}
		else if(state==CREATION) {
			system("clear");
			std::cout << "$$ Chat - Besz. Letrehozasa" << std::endl
					  << "Add meg a felhasznalokat, kilepeshez 'quit'" << std::endl
					  << "----------------------------" << std::endl
					  << "Ha titkos chatet akarsz ' - hide '" << std::endl << std::endl;
		}
	}
	
	/// Start up manager
	/// Methods get called based on input
	/// Prints menu and stuff, will get GUI
	void start() { 
		
		/// Start main menu
		std::string msg;
		std::string msg2;
		
		while( msg!="3" ) {
			
			showmenu();
			std::cin >> msg;
			/// Enter methods if input is correct, change STATE as well
		
				
			/// User wants to join conversations
			/// If given number is correct join him
			/// If not part of conversation or else throw error
			if(msg=="1") {
				state = JOIN;
				
				while(msg2!="quit") {
					
					showmenu();
					/// Print out conversations
					user.get_conversations();
					
					/// Get conversation id
					std::cin >> msg;
					msg2 = msg;
					/// Join conv
					state = CONVERSATION;
					showmenu();
					while(msg!="q") getline(std::cin, msg);
					
					state = JOIN;
				}
				
				state = MAIN;
			}
					
			/// User wants to create new conversation
			/// Give available users and let him do it
			/// Secret chats can be created
			else if(msg=="2") {
				state = CREATION;
				showmenu();
				
				while(msg!="quit") {
					getline(std::cin, msg);
				}
			}
		}
		
			
	}
	
};

#endif
