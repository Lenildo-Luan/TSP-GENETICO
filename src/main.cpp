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
bool compareSolByCost(const tChromosome &data1, const tChromosome &data2);
void showAdjacenciesList(vector< vector< pair<int, int> > > listOfAdjacencies, int father, int mother);

// Genetic algorithm
int construtivo(vector<int> &sol, int depot, float alpha);
void genetic(vector<tChromosome> &pop, int Imax, int popSize);
int doubleBridge(vector<int> &sol, int cost);

// Procedures
void generatePop(vector<tChromosome> &pop, int popSize);
void generateNewPop(vector<tChromosome> &pop);
void crossoverERX(vector<int> &father, vector<int> &mother, tChromosome &children);

//MAIN
int main(int argc, char **argv){
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
void genetic(vector<tChromosome> &pop, int Imax, int popSize){
  int iter = 0;
  tChromosome bestSol;

  generatePop(pop, popSize);
  bestSol = pop[0];
  
  while(iter < 50){
    generateNewPop(pop);

    if(pop[0].cost < bestSol.cost){
      bestSol = pop[0];
      iter = 0;
    } else {
      iter++;
    }
  }
  

  printPop(pop);
}
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

  sort(pop.begin(), pop.end(), compareSolByCost);
} 
void generateNewPop(vector<tChromosome> &pop){
  // Vars
  int randomFatherPosition = 0, randomMotherPosition = 0;
  int insertionPosition = 0;

  // Data structs
  vector<int> fathersDisponibility;
  uniform_int_distribution<int> linear_mutation(0, 100);
  uniform_int_distribution<int> linear_survivors(25, 74);

  // Random numbers
  random_device rd;
  mt19937 mt(rd());

  // Initialize Data Structs
  for(int i = 0; i < pop.size(); i++){
    fathersDisponibility.push_back(i);
  }

  // Reproduce
  while(fathersDisponibility.size() > 1) {
    // Select a random father and mother
    uniform_int_distribution<int> linear_i(0, fathersDisponibility.size() - 1);
    randomFatherPosition = linear_i(mt);
    randomMotherPosition = linear_i(mt);
    if(randomFatherPosition == randomMotherPosition) continue;

    tChromosome children;

    // Generate a children
    crossoverERX(pop[fathersDisponibility[randomFatherPosition]].sol, 
                 pop[fathersDisponibility[randomMotherPosition]].sol, 
                 children);

    // Appli mutation: 5% 
    if(linear_mutation(mt) <= 5) {
      children.cost = doubleBridge(children.sol, children.cost);
      //cout << "MUTATION" << endl;
    }

    // cout << pop[fathersDisponibility[randomMotherPosition]].sol.size() << endl;
    // assert(children.sol.size() == dimension+1);

    // Insert the children
    if(children.cost >= pop[pop.size()-1].cost){
        pop.emplace(pop.end(), children);
        //cout << "POS: " << pop.size()-1 << " COST: " << children.cost << endl;
    } else {
      for(int i = 0; i < pop.size(); i++){
        if(children.cost <= pop[i].cost) {
          pop.emplace(pop.begin() + i, children);
          //cout << "POS: " << i << " COST: " << children.cost << endl;
          break;
        }
      }
    }
    
    //Discart fathers
    fathersDisponibility.erase(fathersDisponibility.begin() + randomFatherPosition);
    fathersDisponibility.erase(fathersDisponibility.begin() + randomMotherPosition - 
                              ((randomFatherPosition < randomMotherPosition) ? 1 : 0));
    
  }
  
  // Select survivors
  for(int i = 0; i < 25; i++){
    int afff = linear_survivors(mt)-i;
    pop.erase(pop.begin() + afff);
    //cout << pop.size() << " " << afff << endl;
  }
}
void crossoverERX(vector<int> &father, vector<int> &mother, tChromosome &children){
  // Vars
  int randomFatherPosition = 0, randomMotherPosition = 0;
  int verticeIndex = 0, bestVertice = 0, newVertice = 0, bestVerticePos = 0;
  int initVertice = 0;

  // Random
  random_device rd;
  mt19937 mt(rd());

  // Data Structs
  vector< vector< pair<int, int> > > listOfAdjacencies;
  vector< pair<int, int> > adjacencies;
  pair<int, int> adjacency;
  vector<int> bestArcs;
  int matAdjIncluded[dimension][dimension];

  // Iniciaaliza estruturas de dados
  std::fill(*matAdjIncluded, *matAdjIncluded + dimension*dimension, 0);
  // for (int i = 0; i < dimension; i++){
  //   cout << "[ ";

  //   for (int j = 0; j < dimension; j++){
  //     cout << matAdjIncluded[i][j] << " ";
  //   }

  //   cout << "]" << endl;
  // }

  for (int i = 0; i < dimension; i++){
    listOfAdjacencies.push_back(adjacencies);
  }

  children.sol.push_back(1);
  children.cost = 0;

  // Cria lista de arestas
  for(int i = 0; i < dimension; i++){
    if(matAdjIncluded[father[i]-1][father[i+1]-1] == 0) {
      adjacency = make_pair(father[i], father[i+1]);
      listOfAdjacencies[father[i]-1].push_back(adjacency);
      matAdjIncluded[father[i]-1][father[i+1]-1]++;

      adjacency = make_pair(father[i+1], father[i]);
      listOfAdjacencies[father[i+1]-1].push_back(adjacency);
      matAdjIncluded[father[i+1]-1][father[i]-1]++;
    }

    if(matAdjIncluded[mother[i]-1][mother[i+1]-1] == 0) {
      adjacency = make_pair(mother[i], mother[i+1]);
      listOfAdjacencies[mother[i]-1].push_back(adjacency);
      matAdjIncluded[mother[i]-1][mother[i+1]-1]++;

      adjacency = make_pair(mother[i+1], mother[i]);
      listOfAdjacencies[mother[i+1]-1].push_back(adjacency);
      matAdjIncluded[mother[i+1]-1][mother[i]-1]++;
    }
  }

  // Mostra resultados
  // for (int i = 0; i < dimension; i++){
  //   cout << "[ ";

  //   for (int j = 0; j < dimension; j++){
  //     cout << matAdjIncluded[i][j] << " ";
  //   }

  //   cout << "]" << endl;
  // }

  // showAdjacenciesList(listOfAdjacencies, randomFatherPosition, randomMotherPosition);
  
  // Gera um filho
  for(int i = 0; i < dimension-1; i++) { (12, 2), (12, 5), (12, 6),
    bestVerticePos = 0;

    for(int j = 0; j < listOfAdjacencies[verticeIndex].size(); j++){
      // cout << "[1]" << listOfAdjacencies[listOfAdjacencies[verticeIndex][j].second-1].size() << endl;
      if(listOfAdjacencies[listOfAdjacencies[verticeIndex][j].second-1].size() > 0) {
        bestVertice = listOfAdjacencies[verticeIndex][j].second;
        bestArcs.push_back(j);
        break;
      }
      bestVerticePos++;
    }
    
    for(int j = bestVerticePos + 1; j < listOfAdjacencies[verticeIndex].size(); j++) {
      newVertice = listOfAdjacencies[verticeIndex][j].second;
      // cout << "[2]" << listOfAdjacencies[newVertice-1].size() << endl;
      if(listOfAdjacencies[newVertice-1].size() == 0) continue;

      if(listOfAdjacencies[newVertice-1].size() < listOfAdjacencies[bestVertice-1].size()) {
        bestVertice = newVertice;
        bestVerticePos = j;
        bestArcs.clear();
        bestArcs.push_back(j);
      } else if(listOfAdjacencies[newVertice-1].size() == listOfAdjacencies[bestVerticePos-1].size()) {
        bestVerticePos = newVertice;
        bestVerticePos = j;
        bestArcs.push_back(j);
      }
    }
    // cout << bestArcs.size() << endl;
    if(bestArcs.size() ==  0){
      uniform_int_distribution<int> linear_j(1, dimension);
      int randomVertice = 0;

      while(1) {
        randomVertice = linear_j(mt);

        if(listOfAdjacencies[randomVertice-1].size() == 0 || randomVertice == verticeIndex + 1) continue;
        else {
          bestVertice = randomVertice;
          break;
        }
      }
    } else {
      uniform_int_distribution<int> linear_j(0, bestArcs.size() - 1);
      bestVerticePos = bestArcs[linear_j(mt)];
      bestVertice = listOfAdjacencies[verticeIndex][bestVerticePos].second;
    }

    // cout << listOfAdjacencies[bestVertice-1].size() << " " << bestVertice << endl;
    // cout <<  "=========================================" << endl;

    bestArcs.clear();
    listOfAdjacencies[verticeIndex].clear();
    children.sol.push_back(bestVertice);
    children.cost += matrizAdj[children.sol[i]][bestVertice];
    verticeIndex = bestVertice-1;
  }

  children.cost += matrizAdj[children.sol[children.sol.size()-1]][1];
  children.sol.push_back(1);

  //Mostra resultados
  // cout << "cost: " << children.cost << endl;
  //printSol(children.sol);
}

// Algorithms
int construtivo(vector<int> &sol, int depot, float alpha){
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
  for(size_t i = 1; i <= dimension; i++){
    if (i == depot) continue;
    verticesList.push_back(i);
  }

  // Escolhe três vertices de forma aleatória
  for(size_t i = 0; i < 3; i++){
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
  while(1){
    sizeSol = sol.size();
    sizeVerticesList = verticesList.size();

    // Gera lista de custo de vertices
    for(size_t i = 0; i < sizeVerticesList; i++){ // Itera sobre vértices restantes
      for(size_t j = 1; j < sizeSol; j++){ // Itera sobre a solução
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
int doubleBridge(vector<int> &sol, int cost){
  random_device rd;
  mt19937_64 mt(rd());
  uniform_int_distribution<int> linear_bme20(2, dimension / 3);
  uniform_int_distribution<int> linear_bma20(2, dimension / 10);

  vector<int> subSeq1, subSeq2;

  int sizeSubSeq1, sizeSubSeq2;
  int initSubSeq1, initSubSeq2;
  int initialCost, finalCost, deltaCost = 0;

  sizeSubSeq1 = linear_bme20(mt);
  sizeSubSeq2 = linear_bme20(mt);

  uniform_int_distribution<int> linear_p1(1, sol.size() - sizeSubSeq1 - 1);
  uniform_int_distribution<int> linear_p2(1, sol.size() - sizeSubSeq2 - 1);

  while (1)
  {
    initSubSeq1 = linear_p1(mt);
    initSubSeq2 = linear_p2(mt);

    if ((initSubSeq2 <= (initSubSeq1 - sizeSubSeq2) && initSubSeq1 > sizeSubSeq2) || (initSubSeq2 >= (initSubSeq1 + sizeSubSeq1) && ((sol.size() - 1) - initSubSeq1 - (sizeSubSeq1 - 1)) > sizeSubSeq2))
      break;
  }

  for (size_t i = 0; i < sizeSubSeq1; i++)
  {
    subSeq1.push_back(sol[initSubSeq1 + i]);
  }

  for (size_t i = 0; i < sizeSubSeq2; i++)
  {
    subSeq2.push_back(sol[initSubSeq2 + i]);
  }

  if (initSubSeq1 > initSubSeq2)
  {
    if ((initSubSeq1 - sizeSubSeq2) == initSubSeq2)
    {
      deltaCost = (matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq1 + sizeSubSeq1]]) -
                  (matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]]);
    }
    else
    {
      initialCost = (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]] + matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]) -
                    (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]] +
                     matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]);

      finalCost = (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq1 + sizeSubSeq1]] +
                   matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq2 + sizeSubSeq2]]) -
                  (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]] + matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]);

      deltaCost = finalCost + initialCost;
    }

    sol.erase(sol.begin() + initSubSeq2, sol.begin() + initSubSeq2 + (sizeSubSeq2));
    sol.insert(sol.begin() + initSubSeq2, subSeq1.begin(), subSeq1.begin() + subSeq1.size());

    sol.erase(sol.begin() + initSubSeq1 + (sizeSubSeq1 - sizeSubSeq2), sol.begin() + initSubSeq1 + (sizeSubSeq1 - sizeSubSeq2) + (sizeSubSeq1));
    sol.insert(sol.begin() + initSubSeq1 + (sizeSubSeq1 - sizeSubSeq2), subSeq2.begin(), subSeq2.begin() + subSeq2.size());
  }
  else
  {
    if (initSubSeq1 + sizeSubSeq1 == initSubSeq2)
    {
      deltaCost = (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq2 + sizeSubSeq2]]) -
                  (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]);
    }
    else
    {
      initialCost = (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]] + matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]) -
                    (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]] +
                     matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]);

      finalCost = (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq2]] + matrizAdj[sol[initSubSeq2 + sizeSubSeq2 - 1]][sol[initSubSeq1 + sizeSubSeq1]] +
                   matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq1]] + matrizAdj[sol[initSubSeq1 + sizeSubSeq1 - 1]][sol[initSubSeq2 + sizeSubSeq2]]) -
                  (matrizAdj[sol[initSubSeq1 - 1]][sol[initSubSeq1 + sizeSubSeq1]] + matrizAdj[sol[initSubSeq2 - 1]][sol[initSubSeq2 + sizeSubSeq2]]);

      deltaCost = finalCost + initialCost;
    }

    sol.erase(sol.begin() + initSubSeq1, sol.begin() + initSubSeq1 + (sizeSubSeq1));
    sol.insert(sol.begin() + initSubSeq1, subSeq2.begin(), subSeq2.begin() + subSeq2.size());

    sol.erase(sol.begin() + initSubSeq2 + (sizeSubSeq2 - sizeSubSeq1), sol.begin() + initSubSeq2 + (sizeSubSeq2 - sizeSubSeq1) + (sizeSubSeq2));
    sol.insert(sol.begin() + initSubSeq2 + (sizeSubSeq2 - sizeSubSeq1), subSeq1.begin(), subSeq1.begin() + subSeq1.size());
  }

  return cost + deltaCost;
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

  cout << "]" << endl << endl;
}
bool compareByCost(const tInsertion &data1, const tInsertion &data2){
  return data1.cost < data2.cost;
}
bool compareSolByCost(const tChromosome &data1, const tChromosome &data2){
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
void custoSolucao(int *custoTotal, vector<int> solucao, int tamanhoArray){
  *custoTotal = 0;

  for (size_t i = 0; i < solucao.size() - 1; i++)
  {
    *custoTotal += matrizAdj[solucao[i]][solucao[i + 1]];
  }
}

// Tests