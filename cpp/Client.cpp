/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:56 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/27 20:24:39 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Client.hpp"

Client::Client(const Client &client){
    *this = client;
}

Client &Client::operator=(const Client &client){
    if (this != &client){
        
    }
    return (*this);
}

Client::Client(int new_fd){
    this->_socket = new_fd;
}
