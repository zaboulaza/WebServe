# commend possible :
    
    // Donc apres un : nc localhost "port"
    je peux recevoir plusieur ligne de commande comme par exemple: 

    ``  GET /upload HTTP/1.1
        Host: localhost
        Content-Length: 11
        Content-Type: text/plain
    ``
    et il peux me les envoier en plusieur fois genre :

    1er = GET /upl
    2eme = oad HTTP/1.1
    3eme = etc...

    exemple :

        // [header]
        POST /upload HTTP/1.1
        Host: localhost

        // [body]
        hello world

    donc je doit faire une boucle sur ---> recv();
    et dans un premier temps recupere le [header] et
    si les condition sont respecter recuperer le [body]

    condition pour stoper la boucle :

    recupere le header :

    stoper si on lit ->

        1- "/n/n"       // sa c'est si on se connecte avec nc 
        2- "/r/n/r/n"   // sa c'est si on se connecte avec curl
    
    apres sa j'ai recuper le header

    mtn on vas voir les condition pour recuperee le [body] :

    --> si la requete est POST et contient :
    Content-lenght = X 

    je lit X cara dans le body 
    
