
 import {Animated, Easing, ImageStyle, StyleProp} from 'react-native';

 const DESELECTED_OPACITY = 0.25;
 const HIGHLIGHTED_OPACITY = 0.5;
 const SELECTED_OPACITY = 1;

 export function createSelectedAnimatedStyles(
   keyframe: Animated.Value,
   rating: number,
   useNativeDriver: boolean,
 ): Animated.WithAnimatedValue<StyleProp<ImageStyle>> {
   const animateRotate = keyframe.interpolate(
     SELECTED_INTERPOLATIONS[rating].rotate,
   );
   const animateScale = keyframe.interpolate(
     SELECTED_INTERPOLATIONS[rating].scale,
   );
   const animateOpacity = keyframe.interpolate(
     SELECTED_INTERPOLATIONS[rating].opacity,
   );

   return {
     transform: [{scale: animateScale}, {rotate: animateRotate}],
     opacity: animateOpacity,
   };
 }

 export function createSelectedAnimations(
   keyframe: Animated.Value,
   starNo: number,
   useNativeDriver: boolean,
 ): Animated.CompositeAnimation {
   return Animated.timing(keyframe, {
     toValue: 1,
     delay: starNo * 40,
     duration: 1000,
     easing: Easing.bounce,
     useNativeDriver,
   });
 }

 export function createHighlightAnimatedStyles(
   keyframe: Animated.Value,
 ): Animated.WithAnimatedValue<StyleProp<ImageStyle>> {
   const animateOpacity = keyframe.interpolate({
     inputRange: [0, 1],
     outputRange: [DESELECTED_OPACITY, HIGHLIGHTED_OPACITY],
   });
   return {
     opacity: animateOpacity,
   };
 }

 export function createHighlightAnimations(
   keyframe: Animated.Value,
   useNativeDriver: boolean,
 ): Animated.CompositeAnimation[] {
   const fadeIn = Animated.timing(keyframe, {
     toValue: 1,
     duration: 200,
     useNativeDriver,
   });

   const fadeOut = Animated.timing(keyframe, {
     toValue: 0,
     duration: 200,
     useNativeDriver,
   });

   return [fadeIn, fadeOut];
 }

 const noStar = {
   opacity: {
     inputRange: [0, 1],
     outputRange: [DESELECTED_OPACITY, DESELECTED_OPACITY],
   },
   rotate: {
     inputRange: [0, 1],
     outputRange: ['0deg', '0deg'],
   },
   scale: {
     inputRange: [0, 1],
     outputRange: [1, 1],
   },
 };

 const oneStar = {
   opacity: {
     inputRange: [0, 0.85],
     outputRange: [DESELECTED_OPACITY, SELECTED_OPACITY],
   },
   rotate: {
     inputRange: [0, 0.4, 0.85, 1],
     outputRange: ['0deg', '5deg', '-3deg', '0deg'],
   },
   scale: {
     inputRange: [0, 1],
     outputRange: [1, 1],
   },
 };

 const twoStar = {
   opacity: {
     inputRange: [0, 1],
     outputRange: [DESELECTED_OPACITY, SELECTED_OPACITY],
   },
   rotate: {
     inputRange: [0, 0.4, 0.85, 1],
     outputRange: ['0deg', '4deg', '-3deg', '0deg'],
   },
   scale: {
     inputRange: [0, 1],
     outputRange: [1, 1],
   },
 };

 const threeStar = {
   opacity: {
     inputRange: [0, 0.6],
     outputRange: [DESELECTED_OPACITY, SELECTED_OPACITY],
   },
   rotate: {
     inputRange: [0, 1],
     outputRange: ['0deg', '0deg'],
   },
   scale: {
     inputRange: [0, 0, 0.6, 1],
     outputRange: [1, 0.9, 1.02, 1],
   },
 };

 const fourStar = {
   opacity: {
     inputRange: [0, 0.6],
     outputRange: [DESELECTED_OPACITY, SELECTED_OPACITY],
   },
   rotate: {
     inputRange: [0, 1],
     outputRange: ['0deg', '0deg'],
   },
   scale: {
     inputRange: [0, 0, 0.6, 1],
     outputRange: [1, 0.8, 1.04, 1],
   },
 };

 const fiveStar = {
   opacity: {
     inputRange: [0, 0.6],
     outputRange: [DESELECTED_OPACITY, SELECTED_OPACITY],
   },
   rotate: {
     inputRange: [0, 1],
     outputRange: ['0deg', '0deg'],
   },
   scale: {
     inputRange: [0, 0, 0.6, 1],
     outputRange: [1, 0.6, 1.08, 1],
   },
 };

 const SELECTED_INTERPOLATIONS: any = {
   0: noStar,
   1: oneStar,
   2: twoStar,
   3: threeStar,
   4: fourStar,
   5: fiveStar,
 };
