
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
topic = 'covid'

if (topic == 'politics'):
    user_topic = 'politics.txt'
    user_mask = 'wolfstencil.jpg'
elif (topic == 'gaming'):
    user_topic = 'Gaming.txt'
    user_mask = 'controller.png'
elif (topic == 'fashion'):
    user_topic = 'Fashion.txt' 
    user_mask = 'dress.jpg'
elif (topic == 'sports'):
    user_topic = 'sports.txt' 
    user_mask = 'football.jpg'
elif (topic == 'covid'):
    user_topic = 'covid.txt'
    user_mask = 'heart.png'

#get user's color preference (black/white or color)
#get user's max words
user_max = 80
color = False #true for color, false for gray scale

text = open(path.join(d, user_topic)).read()

cloud_mask = np.array(Image.open(path.join(d, user_mask)))
wordcloud = WordCloud(mask=cloud_mask, contour_width=3, contour_color='white', max_words=user_max, min_word_length=4).generate(text)
wordcloud.to_file(path.join(d, "wordcloud_created.jpg"))

if (not color):
    plt.imshow(wordcloud.recolor(color_func=grey_color_func, random_state=3), interpolation='bilinear')
else:
    plt.imshow(wordcloud.recolor(color_func=None), interpolation='bilinear')
plt.axis("off")
plt.show()
