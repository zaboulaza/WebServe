/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 13:39:04 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/29 04:23:15 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hpp/Epoll.hpp"

int main(int ac, char **av){

    if (ac < 2){
        std::cerr << "Error : need more arguments" << std::endl;
        return (0);
    }
    
    Epoll epoll;
    Server serv;

    // std::vector<Server>

    epoll.set_port(av, ac);
    if (!epoll.serv_init())
        return (1);

    return 0;
}

// nginx + nom de regle ex : auto_index