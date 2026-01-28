/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 13:39:04 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/29 00:43:39 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hpp/Server.hpp"

int main(int ac, char **av){

    if (ac != 2){
        std::cerr << "Error : need more arguments" << std::endl;
        return (0);
    }
    
    Server serv;

    // std::vector<Server>

    serv.set_port(av[1]);
    if (!serv.serv_init())
        return (1);

    return 0;
}

// nginx + nom de regle ex : auto_index