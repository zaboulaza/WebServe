/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/26 15:16:56 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/26 18:24:01 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(const Client &serv){
    *this = serv;
}

Client &Client::operator=(const Client &serv){
    if (this != &serv){
        
    }
    return (*this);
}

