#include "readData.h"
#include "Util.h"

#include <string>
#include <random>
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

typedef struct
{
  int pos;
  int vertice;
  int cost;
} tInsertion;

typedef struct
{
  vector<int> sol;
  int cost;
} tChromosome;

// Pega da instâncias
double **matrizAdj; // matriz de adjacencia
int dimension;      // quantidade total de vertices

// Untils
void printData();                                                          // Mostra matriz de adjacencia
void printPop(vector<tChromosome> &solucao);                 // Mostra a solução inicial gerada pel algorítimo escolhido
// void custoSolucao(int *custoTotal, vector<int> solucao, int tamanhoArray); // Mostra o custo da solução gerada
bool compareByCost(const tInsertion &data1, const tInsertion &data2);

// Genetic algorithm
int construtivo(vector<int> &sol, int depot, float alpha);
void genetic(vector<tChromosome> &pop, int Imax, int popSize);

// Procedures
void generatePop(vector<tChromosome> &pop, int popSize);

//MAIN
int main(int argc, char **argv)
{
  readData(argc, argv, &dimension, &matrizAdj);
  //printData();

  vector<tChromosome> pop;
  int maxIter = (dimension > 150) ? dimension / 2 : dimension;
  int popSize = 50;

  genetic(pop, maxIter, popSize);

  // cout << "Custo: " << costSol << endl;
  // printSolucao(sol, dimension);

  return 0;
}

//GILS-RVND
int construtivo(vector<int> &sol, int depot, float alpha)
{
  // Inicia variáveis
  vector<int> verticesList;           // Lista de vertices faltando
  vector<tInsertion> bestVerticesList; // Lista dos X melhores verticesList

  random_device rd;
  mt19937 mt(rd()); // Gerador de números aleatórios

  int rBest;
  int randomVertice;
  int sizeSol;
  int sizeVerticesList;
  int costSol = 0;
  tInsertion insertion;

  //Adiciona coleta ao vector
  sol.clear();

  sol.push_back(depot);

  // Gera lista de vertices faltantes
  for (size_t i = 1; i <= dimension; i++)
  {
    if (i == depot)
      continue;
    verticesList.push_back(i);
  }

  // Escolhe três vertices de forma aleatória
  for (size_t i = 0; i < 3; i++)
  {
    uniform_int_distribution<int> linear_i(0, verticesList.size() - 1); // Distribuição linear de inteiros para escolher vertice inicial
    randomVertice = linear_i(mt);

    sol.push_back(verticesList[randomVertice]);
    verticesList.erase(verticesList.begin() + randomVertice);

    costSol += matrizAdj[sol[i]][sol[i + 1]];
  }

  // Adiciona deposito ao vector
  sol.push_back(depot);

  costSol += matrizAdj[sol[3]][sol[4]];

  // Gera solução
  while (1)
  {
    sizeSol = sol.size();
    sizeVerticesList = verticesList.size();

    // Gera lista de custo de vertices
    for (size_t i = 0; i < sizeVerticesList; i++)
    { // Itera sobre vértices restantes
      for (size_t j = 1; j < sizeSol; j++)
      { // Itera sobre a solução
        insertion.vertice = verticesList[i];
        insertion.pos = j;
        insertion.cost = (matrizAdj[sol[j - 1]][verticesList[i]] + matrizAdj[verticesList[i]][sol[j]]) - matrizAdj[sol[j - 1]][sol[j]];
        bestVerticesList.push_back(insertion);
      }
    }

    //Arruma lista pelo valor do custo
    sort(bestVerticesList.begin(), bestVerticesList.end(), compareByCost);

    // Adiciona novo vertice à solução conforme alpha
    uniform_int_distribution<int> linear_i_alpha(0, int((bestVerticesList.size() - 1) * alpha));
    rBest = linear_i_alpha(mt);
    sol.emplace(sol.begin() + bestVerticesList[rBest].pos, bestVerticesList[rBest].vertice);

    //Calcula custo
    costSol += bestVerticesList[rBest].cost;

    // Reseta Vectors
    for (size_t i = 0; i < sizeVerticesList; i++)
    {
      if (bestVerticesList[rBest].vertice == verticesList[i])
      {
        verticesList.erase(verticesList.begin() + i);
        break;
      }
    }
    bestVerticesList.clear();

    // Se não tiverem mais candidatos o loop acaba
    if (sizeVerticesList == 1)
    {
      break;
    }
  }

  return costSol;
}
void genetic(vector<tChromosome> &pop, int Imax, int popSize)
{
  generatePop(pop, popSize);

  printPop(pop);
}

// Procedures
void generatePop(vector<tChromosome> &pop, int popSize){
  //Declara Variáveis
  random_device rd;
  mt19937 mt(rd());
  uniform_real_distribution<float> linear_f(0.0, 0.5); // Distribuição linear de reais para gerar alpha
  float alpha = 0.0;
  int depot = 1;

  for (int i = 0; i < popSize; i++){
    tChromosome chromosome;

    alpha = linear_f(mt);
    chromosome.cost = construtivo(chromosome.sol, depot, alpha);

    pop.push_back(chromosome);
  }
} 

// Untils
void printData(){
  cout << endl << "dimension: " << dimension << endl;
  for (size_t i = 1; i <= dimension; i++){
    for (size_t j = 1; j <= dimension; j++){
      cout << matrizAdj[i][j] << " ";
    }
    cout << endl;
  }
}

void printPop(vector<tChromosome> &pop){
  cout << "POPULAÇAO" << endl << endl;

  for (int i = 0; i < pop.size(); i++){
    cout << "Cromosomo " << i << " - " << pop[i].cost << " : [ ";

    for (int j = 0; j < pop[i].sol.size(); j++){
      cout << pop[i].sol[j] << " ";
    }

    cout << "]" << endl;
  }
}

// void custoSolucao(int *custoTotal, vector<int> solucao, int tamanhoArray)
// {
//   *custoTotal = 0;

//   for (size_t i = 0; i < solucao.size() - 1; i++)
//   {
//     *custoTotal += matrizAdj[solucao[i]][solucao[i + 1]];
//   }
// }

bool compareByCost(const tInsertion &data1, const tInsertion &data2)
{
  return data1.cost < data2.cost;
}