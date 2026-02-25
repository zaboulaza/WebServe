/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 00:55:26 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 15:53:07 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Request.hpp"

Request::Request(const Request &request){
    *this = request;
}

Request &Request::operator=(const Request &request){
    if (this != &request){
        this->_header = request._header;
        this->_method = request._method;
        this->_path = request._path;
        this->_version = request._version;
        this->_content_length = request._content_length;
        this->_body = request._body;
    }
    return(*this);
}

std::vector<std::string> Request::split(std::string str, char delimiter) {
    std::vector<std::string> res;
    size_t pos;
    
    while ((pos = str.find(delimiter)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        if (!token.empty())
            res.push_back(token);
        str.erase(0, pos + 1);
    }
    if (!str.empty())
        res.push_back(str);
    
    return res;
}

std::vector<std::string> Request::split_first(std::string str, char delimiter) {
    std::vector<std::string> res;
    size_t pos;
    
    if ((pos = str.find(delimiter)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        res.push_back(token);
        str.erase(0, pos + 1);
    }
    if (!str.empty())
        res.push_back(str);
    
    return res;
}

bool Request::set_first_line(std::string str){
    
    std::vector<std::string> line;
    line = split(str, ' ');
    
    if (line.size() != 3)
        return (false);
    else if (line[0] != "GET" && line[0] != "POST" && line[0] != "DELETE")
        return (false);
    else if (line[1][0] != '/' || line[1].find("..") != std::string::npos)
        return(false);
    else if (line[2] != "HTTP/1.1" && line[2] != "HTTP/1.0")
        return (false);
    this->_method = line[0];
    this->_path = line[1];
    this->_version = line[2];
    return (true);
}

std::string Request::trim(std::string str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v\"");
    if (start == std::string::npos)
        return ("");
    size_t end = str.find_last_not_of(" \t\n\r\f\v\"");
    return str.substr(start, end - start + 1);
}

bool Request::pars_head(std::string str){

    std::vector<std::string> line;
    line = split_first(str, ':');
    for(size_t i = 0; i < line.size(); i++){
        line[i] = trim(line[i]);
    }
    if (line.size() != 2)
        return (false);
    else if (line[0].empty())
        return (false);
    else if ((line[0] == "Content-Length" || 
          line[0] == "Content-length" || 
          line[0] == "content-length") && !line[1].empty()){
        _content_length = atoi(line[1].c_str());
        return (true);
    }
    else{
        _header[line[0]] = line[1];
    }
    return (true);
}

int Request::parse_header(std::string str){
    
    this->_vec_header = split(str, '\n');
    
    for (size_t i = 0; i < _vec_header.size(); i++){
        
        if (!_vec_header[i].empty() && _vec_header[i][_vec_header[i].size() - 1] == '\r')
            _vec_header[i] = _vec_header[i].substr(0, _vec_header[i].size() - 1);
        if (i == 0){
            if (set_first_line(_vec_header[i]) == false)
                return (-1);
        }
        else if (!_vec_header[i].empty()){
            if (pars_head(_vec_header[i]) == false)
                return (-1);
        }
        else if (_vec_header[i].empty())
            return (1);
    }
    return(1);
}

int Request::validate_header(std::set<std::string> allowed_methods){

    if (allowed_methods.find(_method) == allowed_methods.end())
        return (405);
    if (_method == "POST" && _content_length == -1){
        return (411);
    }
    else if (_content_length > 10485760) { // en gros 10MB
        return (413);
    }
    return (1);
}