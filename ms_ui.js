// populate board with mines

import { TILE_STATUSES, createBoard, marktile, revealTile, checkWin, checkLose, } from "./ms_logic.js";

const BOARD_SIZE = 10
const NUMBER_OF_MINES = 10

const board = createBoard(BOARD_SIZE, NUMBER_OF_MINES)
const boardElement = document.querySelector('.board') //place under 'board' class seen in html

const minesLeftText = document.querySelector('[mine-count]')
const messageText = document.querySelector('.subtext')

boardElement.style.setProperty("--size", BOARD_SIZE)
minesLeftText.textContent = NUMBER_OF_MINES

board.forEach(row => [
    row.forEach(tile => {
        boardElement.append(tile.element)
        
        tile.element.addEventListener('click', () => {
            revealTile(board, tile)
            checkGameState()
        })
        
        tile.element.addEventListener('contextmenu', e => {
            e.preventDefault()
            marktile(tile)
            listMinesLeft()
        })
    })
])

function listMinesLeft() {
    const markedTilesCount = board.reduce((count, row) => {
        return count + row.filter(tile => tile.status === TILE_STATUSES.MARKED).length
    }, 0)

    minesLeftText.textContent = NUMBER_OF_MINES - markedTilesCount
}

function checkGameState() {
    const win = checkWin(board)
    const lose = checkLose(board)

    if (win || lose) {
        boardElement.addEventListener('click', stopProp, {capture: true})
        boardElement.addEventListener('contextmenu', stopProp, {capture: true})
    }


    // display win/lose message
    if (win) {
        messageText.textContent='You Win!'
    }
    if (lose) {
        messageText.textContent='You Lose :('
        board.forEach(row => {
            row.forEach(tile => {
                if (tile.status == TILE_STATUSES.MARKED && tile.mine) marktile(tile)
                if (tile.mine) revealTile(board,tile)
            })
        })
    }

    return
}

function stopProp(e) {
    e.stopImmediatePropagation()
}