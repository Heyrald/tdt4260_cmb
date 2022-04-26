#Init:

Runtime: 53 seconds. See that blurIteration takes 99.05% of the runtime. Can probably cut this down considerably.

1: Changed colourtype if ladder into switch-case. New runtime: 46 seconds. Decision: Keep

2: Changed inner loop to go horizontally in the inner loop instead of going vertically. Moved currentY calculations to the y for loop. New runtime: 40 seconds. Decision: Keep

3: Did the same to the outer loop. New runtime: 35 seconds. Decision: Keep

4: Changed sum to long int from double. New runtime: 72 seconds. Decision: Throw

5: Made the multiplication between imagein->x and CurrentY, and imageOut->x and senterY be multiplied when they are updated. New runtime: 36 seconds. Probably already fixed in compiler. Decision: Keep.

6: Stopped blurring the edges as described in the box blur Wikipedia article. New runtime 23 seconds. Decision: Too many errors.

7: Rewrote blurIteration to not be colour dependent. New runtime: 9.45 seconds. Decision: Keep.

8: Changed double to float. New runtime: 9.45 seconds. Decision: more errors, same time. Throw.

9: Made program multithreaded. New runtime: 10.25 seconds. Decision: Keep. Lower CMB runtime.

10: Stopped summing every surrounding pixel to the sum value, and instead just add the new values, and remove the old. New runtime: 1.75 seconds. Decision: Keep.
