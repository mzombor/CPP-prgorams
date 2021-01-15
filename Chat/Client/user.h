#ifndef USER_H
#define USER_H

////////////////////////////////
/// Header FILE for user class
////////////////////////////////

#include <iostream>
#include <string>
#include <unistd.h>


/// User class
/// Will handle user related objects and tasks
/// WOIP, will get extended later on
class User {
	std::string name;
	std::string pass;
	int ID;
public:
	
	/// Base constructor, initializeds insance
	/// with given credentials
	/// @param n - name
	/// @param p - password
	/// @param i - ID
	User( std::string n="", std::string p="", int i=0 ): name(n), pass(p), ID(i) {}
	
	//// Get conversations and print them on output
	void get_conversations() {
		std::cout << " 12 - Vki1 " << std::endl
				  << " 13 - Vki2 " << std::endl
				  << " 14 - Vki3 " << std::endl
				  << " 15 - Vki4 " << std::endl;
	}
	
	
	/// Join conversation
	/// while user doesn't want to 'quit', keep getting input
	/// while updating with another thread
	/// @param id - conversation getting printed
	bool join() {
		//// Empty for now
		return true;
	}
	
};


#endif
