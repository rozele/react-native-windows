 import {AnimatedTintedIcon} from './TintedIcon';
 import {
   createHighlightAnimatedStyles,
   createHighlightAnimations,
   createSelectedAnimatedStyles,
   createSelectedAnimations,
 } from './StarRating.animations';
 import React, {useEffect, useState} from 'react';
 import {StyleSheet, View} from 'react-native';
 import useAnimatedValue from './useAnimatedValue';

 const IconStar = require('./images/star.png');

 type StarProps = {
   highlighted: boolean;
   rating: number;
   starNumber: number;
   useNativeDriver: boolean;
 };

 function Star({highlighted, rating, starNumber, useNativeDriver}: StarProps) {
   const selectedKeyframe = useAnimatedValue(0);
   const selectedAnimatedStyle = createSelectedAnimatedStyles(
     selectedKeyframe,
     rating,
     useNativeDriver,
   );
   useEffect(() => {
     const selectAnimations = createSelectedAnimations(
       selectedKeyframe,
       starNumber,
       useNativeDriver,
     );
     selectedKeyframe.setValue(0);
     if (rating >= starNumber) {
       selectAnimations.start();
     }
   }, [rating, selectedKeyframe, starNumber]);

   const highlightKeyframe = useAnimatedValue(0);
   const highlightAnimatedStyle = createHighlightAnimatedStyles(
     highlightKeyframe,
   );
   useEffect(() => {
     const [fadeInAnimation, fadeOutAnimation] = createHighlightAnimations(
       highlightKeyframe,
       useNativeDriver,
     );
     if (highlighted) {
       fadeInAnimation.start();
     } else {
       fadeOutAnimation.start();
     }
   }, [highlightKeyframe, highlighted]);

   return (
     <AnimatedTintedIcon
       size={36}
       color={'blue'}
       icon={IconStar}
       style={
         rating >= starNumber ? selectedAnimatedStyle : highlightAnimatedStyle
       }
     />
   );
 }

 type MemoizedStarWrapperProps = StarProps & {
   onStarHover: (starNumber: number) => void;
   onStarPress: (starNumber: number) => void;
 };

 const MemoizedStarWrapper = React.memo(
   ({
     onStarHover,
     onStarPress,
     starNumber,
     highlighted,
     rating,
     useNativeDriver,
   }: MemoizedStarWrapperProps) => {
     return (
       <View
         {...{
            onMouseEnter: () => onStarHover(starNumber),
            onTouchEnd: () => onStarPress(starNumber)
        }}>
         <Star
           highlighted={highlighted}
           rating={rating}
           starNumber={starNumber}
           useNativeDriver={useNativeDriver}
         />
       </View>
     );
   },
   (
     prevProps: MemoizedStarWrapperProps,
     nextProps: MemoizedStarWrapperProps,
   ): boolean =>
     prevProps.highlighted === nextProps.highlighted &&
     prevProps.rating === nextProps.rating,
 );

 type StarRatingProps = {
   rating: number;
   onRatingChange?: (rating: number) => void;
   useNativeDriver: boolean;
 };

 export default function StarRating(props: StarRatingProps) {
   const {rating: initialRating, onRatingChange, useNativeDriver} = props;
   const [rating, setRating] = useState(initialRating);
   const [starsHighlighted, setStarsHighlighted] = useState(initialRating);

   const stars = Array.from({length: 5}, (_, i) => i + 1).map((starNumber) => {
     const highlighted = starsHighlighted >= starNumber;
     return (
       <MemoizedStarWrapper
         key={`star_${starNumber}`}
         onStarHover={setStarsHighlighted}
         onStarPress={(starNumber: number) => {
           setRating(starNumber);
           onRatingChange && onRatingChange(starNumber);
         }}
         starNumber={starNumber}
         highlighted={highlighted}
         rating={rating}
         useNativeDriver={useNativeDriver}
       />
     );
   });

   return (
     <View
       key={useNativeDriver ? 1 : 0}
       style={styles.container}
       accessibilityRole="radiogroup"
       {...{onMouseLeave: () => setStarsHighlighted(rating)}}>
       {stars}
     </View>
   );
 }

 const styles = StyleSheet.create({
   container: {
     flexDirection: 'row',
     alignItems: 'center',
   },
 });
