#!/usr/bin/env python
import sys
import numpy as np
import pandas as pd
from sklearn.neural_network import MLPClassifier
from sklearn.model_selection import cross_val_score
from sklearn.metrics import accuracy_score

if len(sys.argv) != 3:
    print('Requeridos 2 argumentos: arquivo de treino e arquivo de teste')
    sys.exit(1)

# Lê os arquivos
treino = pd.read_csv(sys.argv[1])
teste = pd.read_csv(sys.argv[2])
atributos = np.asarray(treino.columns)
if not np.array_equal(atributos, teste.columns):
    print('Os arquivos possuem atributos distintos')
    sys.exit(1)

# O primeiro argumento é o identificador
id = atributos[0]
# Os outros, exceto o último, são os preditivos
preditivos = atributos[1: - 1]
# E que o atributo alvo é o último
alvo = atributos[-1]
print('Atributo identificador:', id)
print('Atributos preditivos:', ', '.join(preditivos))
print('Atributo alvo:', alvo)
print('Numero de registros de treino:', len(treino))
print('Numero de registros de teste:', len(teste))

# Treino do classificador
clf = MLPClassifier(hidden_layer_sizes=[30, 10], max_iter=500)
clf.fit(treino[preditivos], treino[alvo])

# Teste
respostas = clf.predict(teste[preditivos])
for i, reg in teste.iterrows():
    identificador = reg[id]
    classe_real = reg[alvo]
    classe_predita = respostas[i]
    resposta = 'ACERTOU' if classe_real == classe_predita else 'ERROU'
    print('%s, classe real: %s, classe predita: %s. %s!' %
          (identificador, classe_real, classe_predita, resposta))

print('-------------------------------------')
acc = accuracy_score(teste[alvo], respostas, normalize=True) * 100
print('Acuracia: %0.2f%%' % acc)
