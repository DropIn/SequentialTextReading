/*
 *  std.cpp
 *  CurveMatching
 *
 *  Created by Roy Shilkrot on 1/1/13.
 *  Copyright 2013 MIT. All rights reserved.
 *
 */
#include "std.h"
#include "alphanum.hpp"

bool hasEnding (const std::string &fullString, const std::string &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool hasEndingLower (const string  &fullString_, const string  &_ending)
{
	string fullstring = fullString_, ending = _ending;
	transform(fullString_.begin(),fullString_.end(),fullstring.begin(),::tolower); // to lower
	return hasEnding(fullstring,ending);
}


void open_imgs_dir(const char* dir_name, std::vector<std::string>& images_names) {
	std::vector<std::string> filter; filter.push_back("png"); filter.push_back("jpg");
	std::vector<std::string> v = open_dir(dir_name, filter);
	images_names.clear(); images_names.insert(images_names.begin(),v.begin(),v.end());
}

vector<string> open_dir(const char* dir_name, const char* filter_cstr) {
	vector<string> filter; filter.push_back(filter_cstr);
	return open_dir(dir_name,filter);
}

vector<string> open_dir(const char* dir_name, std::vector<std::string>& filter) {
	if (dir_name == NULL) {
		return vector<string>();
	}
	
	string dir_name_ = string(dir_name);
	vector<string> files_;
	
	DIR *dp;
	struct dirent *ep;     
	dp = opendir (dir_name);
	
	if (dp != NULL)
	{
		while ((ep = readdir (dp))) {
			if (ep->d_name[0] != '.')
				files_.push_back(ep->d_name);
		}
		
		(void) closedir (dp);
	}
	else {
		cerr << ("Couldn't open the directory");
		return vector<string>();
	}
	vector<string> final_files;
	for (unsigned int i=0; i<files_.size(); i++) {
		if (files_[i][0] == '.') { 
			continue;
		}
		for (int f=0; f<filter.size(); f++) {
			if (hasEndingLower(files_[i],filter[f])) {
				final_files.push_back(string(dir_name) + "/" + files_[i]);
				break;
			}
		}
	}
	std::sort(final_files.begin(), final_files.end(), doj::alphanum_less<std::string>());
	return final_files;
}	
