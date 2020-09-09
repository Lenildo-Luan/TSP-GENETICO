#include "readData.h"
#include "Util.h"

#include <string>
#include <random>
#include <bits/stdc++.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <utility> 

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
void printPop(vector<tChromosome> &solucao);   
void printSol(vector<int> &sol);              // Mostra a solução inicial gerada pel algorítimo escolhido
// void custoSolucao(int *custoTotal, vector<int> solucao, int tamanhoArray); // Mostra o custo da solução gerada
bool compareByCost(const tInsertion &data1, const tInsertion &data2);
void showAdjacenciesList(vector< vector< pair<int, int> > > listOfAdjacencies, int father, int mother);

// Genetic algorithm
int construtivo(vector<int> &sol, int depot, float alpha);
void genetic(vector<tChromosome> &pop, int Imax, int popSize);

// Procedures
void generatePop(vector<tChromosome> &pop, int popSize);
void crossoverERX(vector<tChromosome> &pop);

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

//Genetic algorithm
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
void genetic(vector<tChromosome> &pop, int Imax, int popSize){
  generatePop(pop, popSize);
  crossoverERX(pop);

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
void crossoverERX(vector<tChromosome> &pop){
  // TODO Renomear variáveis
  // TODO Reestruturar função para melhorar acoplamento
  int randomFatherPosition = 0, randomMotherPosition = 0;
  int currentVertice = 0, bestArc = 0, newArc = 0, bestArcPos = 0;
  int initVertice = 0;

  random_device rd;
  mt19937 mt(rd());
  uniform_int_distribution<int> linear_i(0, pop.size() - 1);

  vector< vector< pair<int, int> > > listOfAdjacencies;
  vector< pair<int, int> > adjacencies;
  pair<int, int> adjacency;
  vector<int> fathers, children, bestArcs;

  int matAdjIncluded[dimension][dimension];
  std::fill(*matAdjIncluded, *matAdjIncluded + dimension*dimension, 0);

  // Iniciaaliza estruturas de dados
  for (int i = 0; i < dimension; i++){
    cout << "[ ";

    for (int j = 0; j < dimension; j++){
      cout << matAdjIncluded[i][j] << " ";
    }

    cout << "]" << endl;
  }

  for (int i = 0; i < dimension; i++){
    listOfAdjacencies.push_back(adjacencies);
  }
  for (int i = 0; i < pop.size(); i++){
    fathers.push_back(i);
  }
  children.push_back(1);

  // Seleciona dois pais aleatórios
  // while(1) {
  //   randomFatherPosition = linear_i(mt);
  //   randomMotherPosition = linear_i(mt);
  //   if(randomFatherPosition != randomMotherPosition) break;
  // }

  randomFatherPosition = 0;
  randomMotherPosition = 1;

  fathers.erase(fathers.begin() + randomFatherPosition);
  fathers.erase(fathers.begin() + randomMotherPosition);

  // Cria lista de arestas
  for (int i = 0; i < dimension; i++){
    if(matAdjIncluded[pop[randomFatherPosition].sol[i]-1][pop[randomFatherPosition].sol[i+1]-1] == 0) {
      adjacency = make_pair(pop[randomFatherPosition].sol[i], pop[randomFatherPosition].sol[i+1]);
      listOfAdjacencies[pop[randomFatherPosition].sol[i]-1].push_back(adjacency);
      matAdjIncluded[pop[randomFatherPosition].sol[i]-1][pop[randomFatherPosition].sol[i+1]-1]++;

      adjacency = make_pair(pop[randomFatherPosition].sol[i+1], pop[randomFatherPosition].sol[i]);
      listOfAdjacencies[pop[randomFatherPosition].sol[i+1]-1].push_back(adjacency);
      matAdjIncluded[pop[randomFatherPosition].sol[i+1]-1][pop[randomFatherPosition].sol[i]-1]++;
    }

    if(matAdjIncluded[pop[randomMotherPosition].sol[i]-1][pop[randomMotherPosition].sol[i+1]-1] == 0) {
      adjacency = make_pair(pop[randomMotherPosition].sol[i], pop[randomMotherPosition].sol[i+1]);
      listOfAdjacencies[pop[randomMotherPosition].sol[i]-1].push_back(adjacency);
      matAdjIncluded[pop[randomMotherPosition].sol[i]-1][pop[randomMotherPosition].sol[i+1]-1]++;

      adjacency = make_pair(pop[randomMotherPosition].sol[i+1], pop[randomMotherPosition].sol[i]);
      listOfAdjacencies[pop[randomMotherPosition].sol[i+1]-1].push_back(adjacency);
      matAdjIncluded[pop[randomMotherPosition].sol[i+1]-1][pop[randomMotherPosition].sol[i]-1]++;
    }
  }

  // Mostra resultados
  for (int i = 0; i < dimension; i++){
    cout << "[ ";

    for (int j = 0; j < dimension; j++){
      cout << matAdjIncluded[i][j] << " ";
    }

    cout << "]" << endl;
  }

  showAdjacenciesList(listOfAdjacencies, randomFatherPosition, randomMotherPosition);
  
  // Gera um filho
  for(int i = 0; i < dimension-1; i++) { (12, 2), (12, 5), (12, 6),
    bestArcPos = 0;

    for(int j = 0; j < listOfAdjacencies[currentVertice].size(); j++){
      cout << "[1]" << listOfAdjacencies[listOfAdjacencies[currentVertice][j].second-1].size() << endl;
      if(listOfAdjacencies[listOfAdjacencies[currentVertice][j].second-1].size() > 0) {
        bestArc = listOfAdjacencies[currentVertice][j].second;
        bestArcs.push_back(j);
        break;
      }
      bestArcPos++;
    }
    
    for(int j = bestArcPos+1; j < listOfAdjacencies[currentVertice].size(); j++) {
      newArc = listOfAdjacencies[currentVertice][j].second;
      cout << "[2]" << listOfAdjacencies[newArc-1].size() << endl;
      if(listOfAdjacencies[newArc-1].size() == 0) continue;

      if(listOfAdjacencies[newArc-1].size() < listOfAdjacencies[bestArc-1].size()) {
        bestArc = newArc;
        bestArcPos = j;
        bestArcs.clear();
        bestArcs.push_back(j);
      } else if(listOfAdjacencies[newArc-1].size() == listOfAdjacencies[bestArc-1].size()) {
        bestArc = newArc;
        bestArcPos = j;
        bestArcs.push_back(j);
      }
    }
    cout << bestArcs.size() << endl;
    if(bestArcs.size() ==  0){
      uniform_int_distribution<int> linear_j(1, dimension);
      int randomVertice = 0;

      while(1) {
        randomVertice = linear_j(mt);

        if(listOfAdjacencies[randomVertice-1].size() == 0 || randomVertice == currentVertice+1) continue;
        else {
          bestArc = randomVertice;
          break;
        }
      }
    } else {
      uniform_int_distribution<int> linear_j(0, bestArcs.size() - 1);
      bestArcPos = bestArcs[linear_j(mt)];
      bestArc = listOfAdjacencies[currentVertice][bestArcPos].second;
    }

    cout << listOfAdjacencies[bestArc-1].size() << " " << bestArc << endl;
    cout <<  "=========================================" << endl;

    bestArcs.clear();
    listOfAdjacencies[currentVertice].clear();
    children.push_back(bestArc);
    currentVertice = bestArc-1;
  }

  children.push_back(1);

  //Mostra resultados
  printSol(children);
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
  cout << "POPULATION" << endl << endl;

  for (int i = 0; i < pop.size(); i++){
    cout << "Chromosome " << i << " - " << pop[i].cost << " : [ ";

    for (int j = 0; j < pop[i].sol.size(); j++){
      cout << pop[i].sol[j] << " ";
    }

    cout << "]" << endl;
  }
}

void printSol(vector<int> &sol){
  cout << "SOLUTION" << endl << endl;
  cout << "[ ";

  for (int i = 0; i < sol.size(); i++){
    cout << sol[i] << " ";
  }

  cout << "]" << endl;
}

// void custoSolucao(int *custoTotal, vector<int> solucao, int tamanhoArray)
// {
//   *custoTotal = 0;

//   for (size_t i = 0; i < solucao.size() - 1; i++)
//   {
//     *custoTotal += matrizAdj[solucao[i]][solucao[i + 1]];
//   }
// }

bool compareByCost(const tInsertion &data1, const tInsertion &data2){
  return data1.cost < data2.cost;
}

void showAdjacenciesList(vector< vector< pair<int, int> > > listOfAdjacencies, int father, int mother){
  cout << "ADJ LIST - FATHER " << father+1 << " - MOTHER " << mother+1 << endl << endl;

  for (int i = 0; i < listOfAdjacencies.size(); i++){
    cout << "Vertice " << i+1 << ": ";

    for (int j = 0; j < listOfAdjacencies[i].size(); j++){
      cout << "(" << listOfAdjacencies[i][j].first << ", " << listOfAdjacencies[i][j].second << "), ";
    }

    cout << endl;
  }

  cout << endl;
}

void printMatAdjIncluded(int **matAdjIncluded){
  cout << "POPULAÇAO" << endl << endl;

  for (int i = 0; i < dimension; i++){
    cout << "[ ";

    for (int j = 0; j < dimension; j++){
      cout << matAdjIncluded[i][j] << " ";
    }

    cout << "]" << endl;
  }
}
