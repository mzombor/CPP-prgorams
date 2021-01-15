#include "manager.h"
#include <iostream>
#include <string>

//////////////////////////////////
/// Main file for client manager
//////////////////////////////////

/// Gets input from user and tries to start it
int main() {
	
	std::string nev = "";
	std::string jelszo;
	
			
	/// Get user input
	std::cout << "Kerlek add meg a felhasznaloi adataidat" << std::endl 
			  << "Kilepeshez add meg nevnek, hogy quit" << std::endl
			  << "- - - - - - - - - - - - - - - - - - " << std::endl << std::endl 
			  << "Felhasznalonev: ";
			  
	std::cin >> nev;
	std::cout << "Jelszo: ";
	std::cin >> jelszo;

	/// Start client with given input
	try{
		
		Manager manager( nev, jelszo );
		manager.start();
		
	/// Catch error if credentials weren't correct
	} catch( std::out_of_range ) {
		std::cout << "anyad";
	}


}
