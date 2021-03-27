

import os
from os import path
from wordcloud import WordCloud
from PIL import Image
import matplotlib.pyplot as plt
import numpy as np

d = path.dirname(__file__) if "__file__" in locals() else os.getcwd()

text = open(path.join(d, 'constitution.txt')).read()

#cloud_mask = np.array(Image.open(path.join(d, "pigstencil.jpg")))

wordcloud = WordCloud(mask=None, contour_width=3, contour_color='red', max_words=100, min_word_length=4).generate(text)

wordcloud.to_file(path.join(d, "pigconstitution.png"))

plt.imshow(wordcloud, interpolation='bilinear')
plt.axis("off")
plt.show()
