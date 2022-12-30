<h1 align="center"> Todo list software </h1>

<p align="center">
  ESGI Project
</p>

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

## Table of Contents

- [Table of Contents](#table-of-contents)
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Contributors](#contributors)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Introduction

Projet de "todolist" réalisé en C avec la librairie GTK+3.2, la partie graphique est réalisée avec Glade.

## Features

- Ajouter/supprimer des tâches
- Éditer les tâches (nom, description, date de fin, priorité)
- Groupe de tâches pour synchroniser le statut et la date de fin
- Ajouter des projets
- Récupération du calendrier à l'aide de cURL
- Suivi de ses finances
- Calculatrice de base intégrée
- Filtre pour retrouver ses tâches (par date, par priorité, par projet)

## Installation

Ce projet a été réalisé sur Windows avec un environnement **WSL**, il est donc conseiller de l'installer sur une distribution Linux ou de l'installer sur Windows avec un environnement WSL.

```bash
#Upgrade the package list
sudo apt update && sudo apt upgrade -y
sudo apt install build-essential -y

#Install GTK+3.2
sudo apt install libgtk-3-dev

#Install cURL
sudo apt install curl && sudo apt install libcurl4-openssl-dev

#Install Glade
sudo apt install glade

#Install postgresql and libpq
sudo apt install postgresql && sudo apt install libpq-dev

#Configure postgresql
sudo -u postgres createuser projet --superuser -p #Respons11
sudo -u postgres createdb projet-todolist --owner projet

#You can now clone the repository and compile the project
gcc `pkg-config --cflags gtk+-3.0` main.c -o main `pkg-config --libs gtk+-3.0` -rdynamic -I/usr/include/postgresql -lpq -lcurl -Wall

#Run the project
./main
```

## Contributors

[@Jayllyz](https://github.com/Jayllyz) & [@userMeh](https://github.com/userMeh)
