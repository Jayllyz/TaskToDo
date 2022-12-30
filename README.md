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
#Clone the repository and go to the project folder
cd Todo-list-software

#Run the script to install the dependencies
chmod +x install.sh
./install.sh

#Compile and run with CMake
mkdir build && cd build
cmake ../
make
./todolist

#Or compile with gcc and run
gcc `pkg-config --cflags gtk+-3.0` main.c -o main `pkg-config --libs gtk+-3.0` -rdynamic -I/usr/include/postgresql -lpq -lcurl -Wall

./main
```

## Contributors

[@Jayllyz](https://github.com/Jayllyz) & [@userMeh](https://github.com/userMeh)
