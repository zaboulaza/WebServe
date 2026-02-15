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

# donc je doit faire une boucle sur ---> recv()

    et dans un premier temps recupere le [header] et
    si les condition sont respecter recuperer le [body]

    condition pour stoper la boucle :

    recupere le header :

    stoper si on lit ->

        1- "/n/n"       // sa c'est si on se connecte avec nc
        2- "/r/n/r/n"   // sa c'est si on se connecte avec curl
    
    apres sa j'ai recuper le header

    mtn on vas voir les condition pour recuperee le [body] :

# ################################################################

# --> si la requete est POST :

    elle doit contenir :

        Content-lenght = X 
        je lit X cara dans le body

    sinon c'est une erreur si y a pas de Content-lenght (400 bad Request)

# --> si la requete est GET  et DELETE:

    un GET ne doit pas contenir de body;
    mais dans le cas ou y a un body:

    je l'ignore simplement et m'arrete au 
    header; (ou erreur on gere comme on veut)

    si il y a un Content-lenght et qu'il est > 0

    c'est une erreur (400 bad Request) 
    

# #######################################################

# c'est quoi une requete HTTP valide ?

    --> elle doit commencer par :

        <METHOD> <PATH> <VERSION>
        GET /index.html HTTP/1.1

    --> si c'est une methode inconue :

        une autre que les 3 : GET, POST ou DELETE
        ou une pas autorizer dans le fichier de config
        elle commence par une maj les autre methode.
        exemple : **PUT , TEST etc...**

        return (501 Not Implemented)
    
    --> avoir un path valide :

        commencer par un **/**
        return (400 Bad Request)

    
    
    