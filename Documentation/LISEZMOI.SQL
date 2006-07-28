Version pour gpsdrive-1.31
Traduit en français par Jacky FRANCOIS <jfrancois@free.fr>

Depuis la version 1.29 GpsDrive supporte les bases de données SQL.

Bases de données supportées (October 2002): MySQL

Compiler:
---------
Le suport pour MySQL ne peut compiler que si vous avez installer
le package de dévelopement mysql-client et ne eut être utilisé
que si libmysqlclient.so est présent.

Si vous ne voulez pas compiler le support de MySQL, même si les
librairies et les headers sont installés, utilisez l'option 
"--disable-mysql" au moment du configure.

Exécution et initialisation:
----------------------------
Si ce n'est déjà fait, installez le package du serveur mysql pour
votre distribution.

o Démarrez le serveur, habituellement en tapant: /etc/init.d/mysql start

o Pour créer une base de données initiale utilisez le fichier "geoinfo.pl" 
  fourni. Assurez-vous que l'utilisateur choisi dispose des droits nécessaires
  pour créer cette table.

  geoinfo.pl  --create-db --fill-defaults --openstreetmap \
              --db-user=root --db-password=<our mysql root password>
  (ajoutez un nom d'utilisateur si besoin est)

  Vous pouvez remplacer gast@localhost par gast@'%' si vous voulez utiliser
  la base de données avec d'autres noms d'hôtes.


Mise à jour:
------------
Si vous avez déjà la base de données et que vous mettez GpsDrive à jour,
assurez-vous que la structure de votre table est bien comme celle du fichier
create.sql (old). Now use 
geoinfo.pl

Gestion du serveur SQL:
-----------------------

mysqlcc est un bon outil, téléchargez-le à cette adresse:

http://www.mysql.com/downloads/gui-mycc.html

Sécurité:
---------
GpsDrive accède par défaut à la table "waypoints" dans la base de données
"geoinfo" avec le nom d'utilisateur "gast" dont le mot de passe est "gast". 
Si vous voulez changer ces paramètres dans la base de données, effectuez
également la modification dans GpsDrive. Pour celà, éditez le fichier
$HOME/.gpsdrive/gpsdriverc (dbuser,dbpass).

Gardez à l'esprit que le mot de passe pour cette base figure en clair dans
gpsdriverc!!!


Importer des fichiers de aypoints existants:
--------------------------------------------
Le script "wp2sql" permet de créer le fichier .sql à partir des fichiers
de waypoints de GpsDrive.

Adaptez-le à vos besoins!!!!

Tapez ensuite:

mysql -u gast -pgast <wp.sql

Vous pouvez également importe des fichies ASCII avec "mysqlimport".

=================================================================================
IMPORTANT:

Activez l'option 'Utiliser SQL' (dans le GUI) pour utiliser les waypoints de SQL.

=================================================================================

Catégories de waypoint:
-----------------------

Vous pouvez nommer vos catégories de waypoint comme vous l'entendez. Il existe
cependant des catégories prédéfinies, auxquelles sont associés des icones dans
GpsDrive.

WLAN	     : point d'accès WLAN ouvert
WLAN-WEP     : point d'éccès WLAN crypté
Rest	     : un restaurant
McDonalds    : un fast-food
BurgerKing   : un autre fast-food
Hotel	     : un hotel
Shop	     : magasin
Monu	     : Monument, attraction touristique
Speedtrap    : Radar de contôle de vitesse, vous avertis quand vous allez passer 
               devant lui
Nightclub    : Pssst...
Airport	     : Aéroport
Golf	     : Cours de golf
Gasstation   : Station essence
Cafe	     : Café

Les noms ne sont pas sensibles à la casse.

Sauvergarder et restaurer votre base de donnée
----------------------------------------------

Sven a conçu gpssql_backup.sh et gpssql_restore.sh. Ces deux scripts sont simples
d'utilisation. Consultez simplement les commentaires dans ces scripts.
