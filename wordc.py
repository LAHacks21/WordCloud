

import os
from os import path
from wordcloud import WordCloud
from PIL import Image
import matplotlib.pyplot as plt
import numpy as np

d = path.dirname(__file__) if "__file__" in locals() else os.getcwd()

#get user's topic choice from frontend
#get user's image mask choice from frontend

user_topic = 'constitution.txt'
user_mask = None
text = open(path.join(d, user_topic)).read()

cloud_mask = np.array(Image.open(path.join(d, "pigstencil.jpg")))
wordcloud = WordCloud(mask=user_mask, contour_width=3, contour_color='red', max_words=100, min_word_length=4).generate(text)
#wordcloud.to_file(path.join(d, "pigconstitution.png"))

plt.imshow(wordcloud, interpolation='bilinear')
plt.axis("off")
plt.show()
