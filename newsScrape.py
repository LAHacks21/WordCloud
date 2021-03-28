import os
from os import path
import requests
from newspaper import Article
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
import random
from GoogleNews import GoogleNews
from nltk.stem import PorterStemmer
from nltk.tokenize import word_tokenize


def getWordForCategory(topic, links):
    ps = PorterStemmer()
    topicRoot = ps.stem(topic)
    def convert(lst):
        wordList = lst.split()
        for i, j in enumerate( wordList ):
            wordList[i] = j.lower()
        return wordList

    articleWords = list() # aggregated words
    for article in links:
        try:
            articleReqested = Article(article, language="en")
            articleReqested.download()
            articleReqested.parse()
            articleWords.append( convert(articleReqested.text) )
            fileName = topic + ".txt"
            with open(fileName, 'w') as f:
                for article in articleWords:
                    for word in article:
                        rootWord = ps.stem(word)
                        if (rootWord.lower() != topicRoot.lower()):
                            f.write("%s" % word)
                            f.write(" ")
        except:
            print("Couldn't Read File")

    
def googleLinks(topic):
    googlenews = GoogleNews()
    googlenews.set_lang('en')
    googlenews.set_period('1d')
    googlenews.set_encode('utf-8')
    article =  googlenews.get_news(topic)
    links = googlenews.get_links()[:5]
    actualLinks = list()
    for l in links:
        l = "http://" + l
        print(l)
        actualLinks.append( requests.get(l).url ) 
    return actualLinks
    