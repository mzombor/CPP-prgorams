#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>

////////////////////////////////
/// Header FILE for user class
////////////////////////////////

/// User class
/// Will handle user related objects and tasks
/// WOIP, will get extended later on
class User
{
	std::string name;
	std::string pass;
	int ID;
public:

	/// Construct user from user file
	/// @param stream - file
	User( std::string filename )
	{
		std::fstream src(filename);
		src >> name;
		src >> pass;
		src >> ID;
		src.close();
	}

	//// Get conversations and print them on output
	void get_conversations()
	{
	}

	/// Join conversation
	/// while user doesn't want to 'quit', keep getting input
	/// while updating with another thread
	/// @param id - conversation getting printed
	bool join()
	{
		//// Empty for now
		return true;
	}

	/// Getter and setter methods
	/// Get te username of user
	std::string getName() { return name; }
	/// Get password
	std::string getPass() { return pass; }
	/// Get id
	int getID() { return ID; }

	/// Operator
	bool operator == ( const User &rhs ) { return ( rhs.ID == ID ); }

};


#endif
