const path = require('path');

module.exports = (env) => {
    return {
        entry: './index.ts',
        optimization: {
            minimize: false
        },
        devtool: 'inline-source-map',
        module: {
            rules: [
                {
                    test: /\.tsx?$/,
                    use: 'ts-loader',
                    exclude: /node_modules/,
                },
            ],
        },
        resolve: {
            extensions: ['.tsx', '.ts', '.js'],
        },
        output: {
            filename: 'main.js',
            path: path.resolve(env.outdir, 'dist'),
        },
        target: 'web'
    }
};