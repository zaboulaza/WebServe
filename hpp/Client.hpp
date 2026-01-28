/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 20:27:49 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/27 20:27:50 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

class Client{

    public:
    
            Client() {};
            ~Client() {};
            Client(const Client &client);
            Client &operator=(const Client &client);
            
            Client(int new_fd);
            
    private:
        
            int _socket; // la socket du client

};