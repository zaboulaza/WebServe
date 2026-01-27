/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 13:39:04 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/01/27 20:25:54 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hpp/Server.hpp"

int main(int ac, char **av){

    if (ac != 2){
        std::cerr << "Error : need more arguments" << std::endl;
        return (0);
    }
    
    Server serv;

    serv.set_port(av[1]);
    if (!serv.serv_init())
        return (1);
    
    return 0;
}