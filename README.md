# Video Textures

Synthesizing Dynamic Video Streams (Computational Photography, Fall 2020)

Richard Liu

A video texture captures an analysis of a video sequence, and is able to provide a stream of images based on the input video sequence. The aim of these video textures is to generate dynamic content which does not noticeably repeat itself exactly. My primary contribution is an implementation of the techniques described in [Schödl, Arno & Szeliski, Richard & Salesin, David & Essa, Irfan. (2000). Video Textures. Proc. of ACM SIGGRAPH. 2000. 10.1145/344779.345012.](https://www.cc.gatech.edu/cpl/projects/videotexture/SIGGRAPH2000/videotex.pdf) for analyzing a video sequence to extract transition data, and for generating a video stream based on the input video sequence. This project is interesting to me because of its potential applications in games and web pages. Often times games and web pages make use of short video and sound clips, which may become repetitive to the point of annoyance; but the techniques described here have the potential to transform these short sequences into arbitrarily-long audio/video streams. (However, this write-up is only concerned with dealing with video sequences, not audio sequences.)

## Background

The implementation described in this writeup is largely based on the techniques described in [Schödl, Arno & Szeliski, Richard & Salesin, David & Essa, Irfan. (2000). Video Textures. Proc. of ACM SIGGRAPH. 2000. 10.1145/344779.345012.](https://www.cc.gatech.edu/cpl/projects/videotexture/SIGGRAPH2000/videotex.pdf), a paper that is primarily concerned with simple video sequence analysis and generation. The authors have went on to write two more related papers, the first of which is [Schödl, Arno & Essa, Irfan. (2000). Machine Learning for Video-Based Rendering.](https://www.cc.gatech.edu/cpl/projects/videotexture/NIPS2000/nips00.pdf). This paper introduces a special type of video texture that handels alpha-matted sprites paired with velocity information, and uses machine learning methods for analysis and generation of video sequences. The second paper by these authors is [Schödl, Arno & Essa, Irfan. (2002). Controlled animation of video sprites. 121-127. 10.1145/545261.545281.](https://www.cc.gatech.edu/cpl/pubs/PDF/ACM-SCA02.pdf), which improves on the video-texture sprite techniques, especially with respect to the speed of video-transitions computation. This paper also introduces improvements to the previous video texture methods in order to handle changes in perspective and more flexible user-interactive controls.

## Algorithm

Videos can be thought of as a sequence of frames, and the transitions between them. Some transitions are considered "good", for example from frame 1 to 2, or frame 2 to 3, and some transitions may be considered "bad", for example in videos that do not loop well, from frame N (the last frame) to frame 1.

The video texture technique consists of an analysis step, which prepares the input video sequence, and a generation step, which renders the output video sequence. The naive analysis algorithm considers all possible transitions (represented by an NxN matrix), and weights each transition according to the distance between the frames (the L2 norm is used here). Then the generation step takes the analysis, and renders a video sequence based on the resulting weights. However, results using this technique are not good. Consider [a video sequence of a swinging pendulum, that passes the center from Left to Right (1), then from Right to Left (2)](https://www.dropbox.com/sh/bnsccxovscvgi37/AACWomm28nC7eQP60T0J90zCa?dl=0&preview=clock.mp4). Notice that at the center frame, the naive technique might select either a frame in (1) or (2), and will erroneously disregard proper physics.

The first improvement is the use of "dynamics-preservation". During the analysis step: When considering transitions, in addition to comparing frame i with frame j, we also consider nearby frames to frame i, compared with nearby frames to frame j. This is a good improvement, and if we think to the pendulum example, a frame in (1) will be quite different now to a frame in (2) when comparing nearby frames in this way. However, results using this improved technique are still not good. Consider a video sequence which outputs frame N-3, then frame N-2, then frame N-1, then frame N, but then there is no good frame to succeed frame N, resulting in a poor output video sequence.

The final improvement is the use of "future cost analysis". During the analysis step, we represent the "bad"-ness of transitions in a future cost matrix. "Anticipated future cost" describes the "bad"-ness of a transition, not just as a function of the distance between frame i and j, but also factoring in the distance from frame j to its best candidate next-frame. We approach this as Q-learning (see 6.036), and use a quickly-converging approximation to calculate these future costs and improve our results. If we think to the pendulum example, frames N-3 through N will have high costs since no good frames can succeed frame N, and our transition matrix will tend to avoid such "dead ends".

## Implementation

I've implemented a `Texture` C++ class which takes an input video sequence (list of Images) and does both analysis and generation of video sequences. The most difficult part of implementation was following along with the paper at the future cost analysis step, where I believe the authors make a mistake in their Q-learning equation.

### Test Cases

[Input #1: Swinging Clock Video](https://www.dropbox.com/sh/bnsccxovscvgi37/AACWomm28nC7eQP60T0J90zCa?dl=0&preview=clock.mp4)

[Input #2: My Plant Video](https://www.dropbox.com/sh/bnsccxovscvgi37/AACWomm28nC7eQP60T0J90zCa?preview=plant_small_360.mp4)

(Note that all probabilities are scaled so that 0 is gray and 1 is white. All pictures can be found [here](https://www.dropbox.com/sh/bnsccxovscvgi37/AACWomm28nC7eQP60T0J90zCa))

The `Texture` class first calculates simple frame-by-frame distances. Here are the associated probabilities, calculated on Input #1 (shortened).

![](https://i.imgur.com/2KgAUAO.png)
<!-- ![frame by frame probabilities](https://i.imgur.com/2KgAUAO.png) -->

Then we calculate the dynamics-preserving distances. Here are the associated probabilities, calculated on Input #1 (shortened).

![](https://i.imgur.com/ZmlYCo5.png)
<!-- ![dynamics-preserving probabilities](https://i.imgur.com/ZmlYCo5.png) -->

Then we calculate the future-cost-analysis distances. Here are the associated probabilities, calculated on Input #1.

![](https://i.imgur.com/kWw4wfR.png)
<!-- ![future-cost-analysis probabilities](https://i.imgur.com/kWw4wfR.png) -->

Then we prepare the analysis for video generation, selecting locally-optimal transitions and pruning transitions below a threshold. Here are the associated probabilities, calculated on Inputs #1 and #2, respectively.

![](https://i.imgur.com/gT3KgnQ.png)
<!-- ![pruned-transitions probabilities for Input #1](https://i.imgur.com/gT3KgnQ.png) -->

![](https://i.imgur.com/eFnF1T5.png)
<!-- ![pruned-transitions probabilities for Input #2](https://i.imgur.com/eFnF1T5.png) -->

### Results

Inputs #1 and #2 are 67 and 148 frames long, respectively. The following are the respective outputs, each generated to be 600 frames long using the random-play technique (so we expect that they do not necessarily loop well).

[Output #1: Random Play Clock Video](https://www.dropbox.com/sh/bnsccxovscvgi37/AADbiqSsX-DXVya2KjYeUJ1ha/randomPlay_clock.mp4?dl=0)

[Output #2: Random Play Plant Video](https://www.dropbox.com/sh/bnsccxovscvgi37/AACiityHJ9yc6vUD3YSF6avia/randomPlay_plant.mp4?dl=0)
