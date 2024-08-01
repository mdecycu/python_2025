import gym

import matplotlib.pyplot as plt
import matplotlib.image as mpimg

env = gym.make('Pong-v0')
env.reset()
env.env.ale.saveScreenPNG(b'test_image.png')
img = mpimg.imread('test_image.png')
plt.imshow(img)
plt.show()

