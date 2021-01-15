#ifndef CONVERSATION_H
#define CONVERSATION_H

#include "user.h"
#include <iostream>
#include <vector>
#include <fstream>

////////////////////////////////////////
/// Header FILE for conversation class
////////////////////////////////////////

/// Conversation class
/// Stores users in a list
/// Write messages intu a file and can be constructed from it
class Conversation
{
	/// File pointer to the conversations holder
	std::string src;
	/// ID of conversation
	int ID;
	/// Messagecount
	int cnt = 0;
	/// List of users in conversation
	std::vector<std::string> userlist;
public:

	/// Base constructor
	/// Created object from the source file
	Conversation( std::string filename )
	{
		src = filename;
		std::fstream convfile(src);
		/// Open file to build conversation
		/// Set object parameters
		convfile >> ID;
		/// Get users
		std::string line;
		while( line != "[MESSAGES]" )
		{
			convfile >> line;
			if( line != "[MESSAGES]" ) userlist.push_back(line);
		}
		/// Get number of messages
		while( convfile >> line ) cnt+=1;
		cnt/=3;
		convfile.close();
	}

	/// Base destructor
	~Conversation() { }

	/// Getter and setter methods
	/// Get IF of conversation
	int getId() { return ID; }

	/// Add message to conversation
	/// Writes message into src file with next count
	/// @param msg - messag being added
	void addmessage( std::string msg )
	{
		std::fstream convfile(src, std::ios::app);
		convfile << ++cnt << " - " << msg << std::endl;
		convfile.close();
	}

};

#endif
