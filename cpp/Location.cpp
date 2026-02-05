/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zaboulaza <zaboulaza@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 16:42:14 by zaboulaza         #+#    #+#             */
/*   Updated: 2026/02/05 06:35:10 by zaboulaza        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../hpp/Location.hpp"

Location::Location(const Location &location) {
    *this = location;
}

Location &Location::operator=(const Location &location) {
    if (this != &location) {
        this->_root = location._root;
        this->_index = location._index;
        this->_allowed_methods = location._allowed_methods;
        this->_auto_index = location._auto_index;
        this->_cgi = location._cgi;
        this->_error_pages = location._error_pages;
        this->_upload_folder = location._upload_folder;
        this->_redirect = location._redirect;
        this->_is_good = location._is_good;
    }
    return *this;
}
