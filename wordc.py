
import os
from os import path
from wordcloud import WordCloud
from PIL import Image
import matplotlib.pyplot as plt
import numpy as np
import random

def grey_color_func(word, font_size, position, orientation, random_state=None, **kwargs):
    return "hsl(0, 0%%, %d%%)" % random.randint(60, 100)

d = path.dirname(__file__) if "__file__" in locals() else os.getcwd()

#get user's topic choice from frontend
#get user's image mask choice from frontend
#get user's color preference (black/white or color)
#get user's max words

#dictionary? 
topics = {'politics': 'politicsArticles.txt', 'gaming':'Gaming.txt', 'placeholder2':'placeholder2.txt', 'placeholder3':'placeholder3.txt'}
stencils = {'none': None, 'wolf': 'wolfstencil.jpg', 'pig': 'pigstencil.jpg', 'placeholder1':'placeholder1.jpg'}

user_topic = 'Gaming.txt' #use dictionary to do this instead
user_mask = 'wolfstencil.jpg' #use dictionary to do this instead
user_max = 80
color_choice = None #or grey_color_func if they choose b/w
text = open(path.join(d, user_topic)).read()

cloud_mask = np.array(Image.open(path.join(d, user_mask)))
wordcloud = WordCloud(mask=cloud_mask, contour_width=3, contour_color='white', max_words=user_max, min_word_length=4).generate(text)
#wordcloud.to_file(path.join(d, "pigconstitution.png"))

plt.imshow(wordcloud.recolor(color_func=None), interpolation='bilinear')
plt.axis("off")
plt.show()
