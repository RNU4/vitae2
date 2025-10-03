function sumOfNaturalNumbers(n) {
    let sum = 0;
    for (let i = 1; i <= n; i++) {
        sum += i;
    }
    return sum;
}

function benchmark(n) {
    const startTime = performance.now();
    const result = sumOfNaturalNumbers(n);
    const endTime = performance.now();
    console.log(`Sum of first ${n} natural numbers = ${result}`);
    console.log(`Time taken: ${((endTime - startTime) / 1000).toFixed(2)} seconds`);
}

const n = 200000000;



const startTime = performance.now();
let sum = 0;
for (let i = 0; i < 200000000; i++) {
    sum += i;
}
const endTime = performance.now();

console.log(`Time taken: ${((endTime - startTime) / 1000).toFixed(2)} seconds`);

function benchmark(n) {
    const startTime = performance.now();
    const result = sumOfNaturalNumbers(n);
    const endTime = performance.now();
    console.log(`Sum of first ${n} natural numbers = ${result}`);
    console.log(`Time taken: ${((endTime - startTime) / 1000).toFixed(2)} seconds`);
}

const n = 200000000;
benchmark(n);
