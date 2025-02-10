# ChimeraHash512

Bienvenue dans le projet **ChimeraHash512** !  
Ce projet implémente une fonction de hachage expérimentale, inspirée par des concepts cryptographiques tels que les S-Boxes, le mélange chaotique et les rounds de Feistel. L'objectif est de produire un condensat de 512 bits (64 octets) en exploitant un bloc de 128 octets.

---

## 📑 **Table des matières**
- [Présentation](#présentation)
- [Fonctionnalités](#fonctionnalités)
- [Architecture et Algorithme](#architecture-et-algorithme)
- [Installation et Compilation](#installation-et-compilation)
- [Utilisation](#utilisation)
- [Limites et Remarques](#limites-et-remarques)
- [Auteur](#auteur)

---

## 🛠️ **Présentation**

Ce projet a été réalisé dans un cadre autodidacte.  
L’objectif principal était d’expérimenter différentes techniques cryptographiques pour mieux comprendre leur fonctionnement, tout en proposant une fonction de hachage originale et sécurisé.  
**⚠️ Ce projet est expérimental et ne doit pas être utilisé en production.**

---

## 🚀 **Fonctionnalités**
- **Bloc de 128 octets** : Chaque bloc est divisé en deux moitiés de 64 octets, combinées via XOR.
- **SBOX personnalisée** : Transformation non-linéaire des octets inspirée d’AES.
- **Mélange chaotique** : Utilisation d’une matrice de type MixColumns.
- **Rotation circulaire** : Rotation de l’état pour augmenter la diffusion.
- **Rounds de Feistel** : Plusieurs itérations pour rendre le hachage complexe.
- **Padding standard** : Ajout de padding si la taille des données n’est pas un multiple du bloc.

---

## ⚙️ **Architecture et Algorithme**

Processus de hashage :

1. **Initialisation** : Un état initial constant de 64 octets est défini.
2. **Compression** :
   - Un XOR est effectué entre la première et la seconde moitié du bloc.
   - Chaque round applique :
     - **SBOX** : Transformation non-linéaire
     - **Mélange Chaotique** : Confusion 
     - **Rotation circulaire** 
     - **Round Feistel** 
3. **Padding** :
   - Un bit `1` suivi de `0` est ajouté.
   - La longueur des données (en bits) est stockée dans les 8 derniers octets du dernier bloc.
4. **Finalisation** : Le dernier bloc est compressé pour obtenir le condensat final de 512 bits.

---

## 🛠️ **Installation et Compilation**

### Prérequis
- Un compilateur C++ 
- **CMake**

  J'ai personnelement utilisé **VsCode**
  
