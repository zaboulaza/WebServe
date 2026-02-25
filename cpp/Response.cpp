/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:11:29 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 18:10:50 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Response.hpp"
#include "../hpp/Server.hpp"
#include "../hpp/Request.hpp"

void Response::handle_erreur_response(int err_code){

    std::string err_message;
    
    if (err_code == 411)
        err_message = "HTTP/1.1 411 Length Required\r\n\r\n";
    else if (err_code == 413)
        err_message = "HTTP/1.1 413 Payload Too Large\r\n\r\n";
    else if (err_code == 400)
        err_message = "HTTP/1.1 400 Bad Request\r\n\r\n";
    else if (err_code == 404)
        err_message = "HTTP/1.1 404 Not Found\r\n\r\n";
    else if (err_code == 405)
        err_message = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    
    send(_socket_client, err_message.c_str(), err_message.size(), 0);
}

int Response::handle_GET_response(Server &server, Request &request){
    
    std::ifstream ifs;
    std::string path;
    std::string body;
    std::string response;

    if (request.get_path() == "/")
        path = server.get_root() + "/" + server.get_index();
    else
        path = server.get_root() + request.get_path();
    
    ifs.open(path.c_str(), std::ifstream::in);
    if (!ifs){
        handle_erreur_response(404);
        return (-1);
    }
    char c;
    while(ifs.get(c))
        body += c;
    ifs.close();
    
    std::stringstream ss; 
    ss << body.size();
    
    response = request.get_version() + " 200 OK\r\n";
    response += "Content-Length: " + ss.str() + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "\r\n";
    response += body;
    
    send(_socket_client, response.c_str(), response.size(), 0);
    return (1);
}

void Response::response_http(Server &server , Request &request){
    
    std::string response;
    
    // response += request.get_version();
    // response += " 200 OK\r\n";
    // response += "Content-Length: 11\r\n";
    // response += "Content-Type: text/html\r\n";
    // response += "\r\n";
    
    if (request.get_method() == "GET")
        handle_GET_response(server, request);

}