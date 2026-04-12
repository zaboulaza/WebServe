/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 13:39:04 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/25 14:41:12 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hpp/Config.hpp"
#include "hpp/Epoll.hpp"

int main(int ac, char **av){

    if (ac < 2){
        std::cerr << "Error : need more arguments" << std::endl;
        return (0);
    }

    Config config;
    if (config.parse(av, ac) == -1){
        std::cerr << "Error : set ports" << std::endl;
        return (0);
    }

    Epoll epoll;
    epoll.set_servers(config.get_servers());
    if (epoll.init_epoll_servers() == -1){
        return (0);
    }

    return (1);
}
