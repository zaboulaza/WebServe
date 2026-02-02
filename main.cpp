/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 13:39:04 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/02 03:14:35 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hpp/Epoll.hpp"

int main(int ac, char **av){

    if (ac < 2){
        std::cerr << "Error : need more arguments" << std::endl;
        return (0);
    }

    Epoll epoll;

    epoll.set_ports(av, ac);
    if (epoll.init_epoll_servers() == -1){
        return (0);
    }
    
    return (1);
}